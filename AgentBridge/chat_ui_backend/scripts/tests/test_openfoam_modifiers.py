import re
import unittest

from scripts.simagent_core.models import ReferenceCase, ReferenceFile
from scripts.simagent_core.solvers.openfoam.capabilities import supported_change_keys
from scripts.simagent_core.solvers.openfoam.modifiers import apply_reference_modifications


CONTROL_DICT_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    object      controlDict;
}

application     icoFoam;
startFrom       startTime;
startTime       0;
stopAt          endTime;
endTime         0.5;
deltaT          0.005;
writeControl    timeStep;
writeInterval   20;
writeFormat     ascii;
writePrecision  6;
writeCompression off;
timeFormat      general;
timePrecision   6;
adjustTimeStep  no;
maxCo           1;
maxDeltaT       1;
runTimeModifiable true;
"""


CONTROL_DICT_KEYS = {
    "write_format": ("writeFormat", "binary"),
    "write_precision": ("writePrecision", "10"),
    "write_compression": ("writeCompression", "on"),
    "adjust_time_step": ("adjustTimeStep", "yes"),
    "max_co": ("maxCo", "0.5"),
    "max_delta_t": ("maxDeltaT", "0.01"),
}


PHYSICAL_PROPERTIES_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    object      physicalProperties;
}

nu              [0 2 -1 0 0 0 0] 0.01;
rho             [1 -3 0 0 0 0 0] 1;
"""


PHYSICAL_PROPERTIES_WITHOUT_RHO_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    object      physicalProperties;
}

nu              [0 2 -1 0 0 0 0] 0.01;
"""


U_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       volVectorField;
    object      U;
}

dimensions      [0 1 -1 0 0 0 0];

internalField   uniform (0 0 0);

boundaryField
{
    movingWall
    {
        type            fixedValue;
        value           uniform (1 0 0);
    }

    fixedWalls
    {
        type            noSlip;
    }

    frontAndBack
    {
        type            empty;
    }
}
"""


U_WITHOUT_MOVING_WALL_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       volVectorField;
    object      U;
}

dimensions      [0 1 -1 0 0 0 0];

internalField   uniform (0 0 0);

boundaryField
{
    fixedWalls
    {
        type            noSlip;
    }

    frontAndBack
    {
        type            empty;
    }
}
"""


U_MOVING_WALL_WITHOUT_VALUE_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       volVectorField;
    object      U;
}

dimensions      [0 1 -1 0 0 0 0];

internalField   uniform (0 0 0);

boundaryField
{
    movingWall
    {
        type            fixedValue;
    }

    fixedWalls
    {
        type            noSlip;
    }
}
"""


PITZ_DAILY_U_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       volVectorField;
    object      U;
}

dimensions      [0 1 -1 0 0 0 0];

internalField   uniform (0 0 0);

boundaryField
{
    inlet
    {
        type            fixedValue;
        value           uniform (10 0 0);
    }

    outlet
    {
        type            zeroGradient;
    }
}
"""


PITZ_DAILY_U_WITHOUT_INLET_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       volVectorField;
    object      U;
}

dimensions      [0 1 -1 0 0 0 0];

internalField   uniform (0 0 0);

boundaryField
{
    outlet
    {
        type            zeroGradient;
    }
}
"""


PITZ_DAILY_U_INLET_WITHOUT_VALUE_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       volVectorField;
    object      U;
}

dimensions      [0 1 -1 0 0 0 0];

internalField   uniform (0 0 0);

boundaryField
{
    inlet
    {
        type            fixedValue;
    }

    outlet
    {
        type            zeroGradient;
    }
}
"""


PITZ_DAILY_P_CONTENT = """FoamFile
{
    version     2.0;
    format      ascii;
    class       volScalarField;
    object      p;
}

dimensions      [0 2 -2 0 0 0 0];

internalField   uniform 0;

boundaryField
{
    inlet
    {
        type            zeroGradient;
    }

    outlet
    {
        type            fixedValue;
        value           uniform 0;
    }
}
"""


def control_dict_reference_file() -> ReferenceFile:
    return ReferenceFile(
        path="case/system/controlDict",
        role="control",
        format="openfoam-dict",
        content=CONTROL_DICT_CONTENT,
    )


def physical_properties_reference_file() -> ReferenceFile:
    return ReferenceFile(
        path="case/constant/physicalProperties",
        role="physical_properties",
        format="openfoam-dict",
        content=PHYSICAL_PROPERTIES_CONTENT,
    )


def physical_properties_without_rho_reference_file() -> ReferenceFile:
    return ReferenceFile(
        path="case/constant/physicalProperties",
        role="physical_properties",
        format="openfoam-dict",
        content=PHYSICAL_PROPERTIES_WITHOUT_RHO_CONTENT,
    )


def u_reference_file(content: str = U_CONTENT) -> ReferenceFile:
    return ReferenceFile(
        path="case/0/U",
        role="initial_condition",
        format="openfoam-dict",
        content=content,
    )


def p_reference_file(content: str = PITZ_DAILY_P_CONTENT) -> ReferenceFile:
    return ReferenceFile(
        path="case/0/p",
        role="initial_condition",
        format="openfoam-dict",
        content=content,
    )


class OpenFOAMModifierTests(unittest.TestCase):
    def test_control_dict_assignment_modifiers_apply_requested_values(self) -> None:
        changes = {
            "write_format": "binary",
            "write_precision": "10",
            "write_compression": "on",
            "adjust_time_step": "yes",
            "max_co": "0.5",
            "max_delta_t": "0.01",
        }

        content, modifications = apply_reference_modifications(control_dict_reference_file(), changes)

        for change_key, (foam_key, value) in CONTROL_DICT_KEYS.items():
            self.assertRegex(content, rf"(?m)^\s*{re.escape(foam_key)}\s+{re.escape(value)};", msg=foam_key)
            self.assertIn(change_key, modifications)

    def test_control_dict_assignment_modifiers_only_modify_target_keys(self) -> None:
        content, modifications = apply_reference_modifications(
            control_dict_reference_file(),
            {"write_format": "binary"},
        )

        self.assertIn("writeFormat     binary;", content)
        self.assertIn("writePrecision  6;", content)
        self.assertIn("writeCompression off;", content)
        self.assertEqual(["write_format"], modifications)

    def test_control_dict_assignment_modifiers_do_not_touch_other_files(self) -> None:
        reference_file = ReferenceFile(
            path="case/system/fvSchemes",
            role="numerics",
            format="openfoam-dict",
            content=CONTROL_DICT_CONTENT,
        )

        content, modifications = apply_reference_modifications(
            reference_file,
            {"write_format": "binary"},
        )

        self.assertEqual(CONTROL_DICT_CONTENT, content)
        self.assertEqual([], modifications)

    def test_control_dict_assignment_modifiers_skip_missing_values(self) -> None:
        content, modifications = apply_reference_modifications(
            control_dict_reference_file(),
            {"write_format": ""},
        )

        self.assertEqual(CONTROL_DICT_CONTENT, content)
        self.assertEqual([], modifications)

    def test_control_dict_assignment_modifiers_accept_alias_values(self) -> None:
        content, modifications = apply_reference_modifications(
            control_dict_reference_file(),
            {"writeFormat": "binary", "maxCo": "0.25"},
        )

        self.assertIn("writeFormat     binary;", content)
        self.assertIn("maxCo           0.25;", content)
        self.assertIn("write_format", modifications)
        self.assertIn("max_co", modifications)

    def test_global_capabilities_include_control_dict_runtime_keys(self) -> None:
        reference_case = ReferenceCase(
            case_name="cavity",
            case_domain="incompressible",
            case_category="cavity",
            case_solver="icoFoam",
        )

        supported = supported_change_keys(reference_case)

        for change_key in CONTROL_DICT_KEYS:
            self.assertIn(change_key, supported)

    def test_dimensioned_scalar_modifier_updates_nu_value_and_preserves_dimensions(self) -> None:
        content, modifications = apply_reference_modifications(
            physical_properties_reference_file(),
            {"kinematic_viscosity": "1e-05"},
        )

        self.assertRegex(content, r"(?m)^\s*nu\s+\[0 2 -1 0 0 0 0\]\s+1e\-05;")
        self.assertIn("rho             [1 -3 0 0 0 0 0] 1;", content)
        self.assertEqual(["kinematic_viscosity"], modifications)

    def test_dimensioned_scalar_modifier_accepts_nu_alias(self) -> None:
        content, modifications = apply_reference_modifications(
            physical_properties_reference_file(),
            {"nu": "0.001"},
        )

        self.assertRegex(content, r"(?m)^\s*nu\s+\[0 2 -1 0 0 0 0\]\s+0\.001;")
        self.assertEqual(["kinematic_viscosity"], modifications)

    def test_dimensioned_scalar_modifier_does_not_touch_other_files(self) -> None:
        reference_file = ReferenceFile(
            path="case/constant/transportProperties",
            role="physical_properties",
            format="openfoam-dict",
            content=PHYSICAL_PROPERTIES_CONTENT,
        )

        content, modifications = apply_reference_modifications(
            reference_file,
            {"kinematic_viscosity": "1e-05"},
        )

        self.assertEqual(PHYSICAL_PROPERTIES_CONTENT, content)
        self.assertEqual([], modifications)

    def test_dimensioned_scalar_modifier_skips_missing_values(self) -> None:
        content, modifications = apply_reference_modifications(
            physical_properties_reference_file(),
            {"kinematic_viscosity": ""},
        )

        self.assertEqual(PHYSICAL_PROPERTIES_CONTENT, content)
        self.assertEqual([], modifications)

    def test_density_modifier_updates_rho_value_and_preserves_dimensions(self) -> None:
        content, modifications = apply_reference_modifications(
            physical_properties_reference_file(),
            {"density": "998.2"},
        )

        self.assertRegex(content, r"(?m)^\s*rho\s+\[1 -3 0 0 0 0 0\]\s+998\.2;")
        self.assertIn("nu              [0 2 -1 0 0 0 0] 0.01;", content)
        self.assertEqual(["density"], modifications)

    def test_density_modifier_accepts_rho_alias(self) -> None:
        content, modifications = apply_reference_modifications(
            physical_properties_reference_file(),
            {"rho": "1.225"},
        )

        self.assertRegex(content, r"(?m)^\s*rho\s+\[1 -3 0 0 0 0 0\]\s+1\.225;")
        self.assertEqual(["density"], modifications)

    def test_density_modifier_does_not_touch_other_files(self) -> None:
        reference_file = ReferenceFile(
            path="case/constant/transportProperties",
            role="physical_properties",
            format="openfoam-dict",
            content=PHYSICAL_PROPERTIES_CONTENT,
        )

        content, modifications = apply_reference_modifications(
            reference_file,
            {"density": "998.2"},
        )

        self.assertEqual(PHYSICAL_PROPERTIES_CONTENT, content)
        self.assertEqual([], modifications)

    def test_density_modifier_skips_when_rho_is_missing(self) -> None:
        content, modifications = apply_reference_modifications(
            physical_properties_without_rho_reference_file(),
            {"density": "998.2"},
        )

        self.assertEqual(PHYSICAL_PROPERTIES_WITHOUT_RHO_CONTENT, content)
        self.assertEqual([], modifications)

    def test_incompressible_capabilities_include_density_but_not_dynamic_viscosity(self) -> None:
        reference_case = ReferenceCase(
            case_name="cavity",
            case_domain="incompressible",
            case_category="cavity",
            case_solver="icoFoam",
        )

        supported = supported_change_keys(reference_case)

        self.assertIn("density", supported)
        self.assertNotIn("dynamic_viscosity", supported)

    def test_boundary_uniform_value_modifier_updates_lid_velocity(self) -> None:
        content, modifications = apply_reference_modifications(
            u_reference_file(),
            {"lid_velocity": "2"},
        )

        self.assertRegex(content, r"(?ms)movingWall\s*\{.*?value\s+uniform\s+\(2 0 0\);.*?\}")
        self.assertEqual(["lid_velocity"], modifications)

    def test_boundary_uniform_value_modifier_preserves_vector_input(self) -> None:
        content, modifications = apply_reference_modifications(
            u_reference_file(),
            {"lid_velocity": "(2 0 0)"},
        )

        self.assertRegex(content, r"(?ms)movingWall\s*\{.*?value\s+uniform\s+\(2 0 0\);.*?\}")
        self.assertEqual(["lid_velocity"], modifications)

    def test_boundary_uniform_value_modifier_does_not_modify_other_patches(self) -> None:
        content, modifications = apply_reference_modifications(
            u_reference_file(),
            {"lid_velocity": "2"},
        )

        self.assertRegex(content, r"(?ms)fixedWalls\s*\{.*?type\s+noSlip;.*?\}")
        self.assertRegex(content, r"(?ms)frontAndBack\s*\{.*?type\s+empty;.*?\}")
        self.assertNotRegex(content, r"(?ms)fixedWalls\s*\{.*?value\s+uniform")
        self.assertNotRegex(content, r"(?ms)frontAndBack\s*\{.*?value\s+uniform")
        self.assertEqual(["lid_velocity"], modifications)

    def test_boundary_uniform_value_modifier_skips_when_patch_is_missing(self) -> None:
        content, modifications = apply_reference_modifications(
            u_reference_file(U_WITHOUT_MOVING_WALL_CONTENT),
            {"lid_velocity": "2"},
        )

        self.assertEqual(U_WITHOUT_MOVING_WALL_CONTENT, content)
        self.assertEqual([], modifications)

    def test_boundary_uniform_value_modifier_skips_when_value_is_missing(self) -> None:
        content, modifications = apply_reference_modifications(
            u_reference_file(U_MOVING_WALL_WITHOUT_VALUE_CONTENT),
            {"lid_velocity": "2"},
        )

        self.assertEqual(U_MOVING_WALL_WITHOUT_VALUE_CONTENT, content)
        self.assertEqual([], modifications)

    def test_boundary_uniform_value_modifier_preserves_brace_structure(self) -> None:
        content, modifications = apply_reference_modifications(
            u_reference_file(),
            {"lid_velocity": "2"},
        )

        self.assertEqual(content.count("{"), content.count("}"))
        self.assertEqual(1, len(re.findall(r"(?m)^\s*boundaryField\s*$", content)))
        self.assertIn("movingWall", content)
        self.assertIn("fixedWalls", content)
        self.assertIn("frontAndBack", content)
        self.assertEqual(["lid_velocity"], modifications)

    def test_inlet_velocity_modifies_u_inlet_value(self) -> None:
        content, modifications = apply_reference_modifications(
            u_reference_file(PITZ_DAILY_U_CONTENT),
            {"inlet_velocity": "10"},
        )

        self.assertRegex(content, r"(?ms)inlet\s*\{.*?value\s+uniform\s+\(10 0 0\);.*?\}")
        self.assertEqual(["inlet_velocity"], modifications)

    def test_inlet_velocity_preserves_vector_input(self) -> None:
        content, modifications = apply_reference_modifications(
            u_reference_file(PITZ_DAILY_U_CONTENT),
            {"inlet_velocity": "(12 0 0)"},
        )

        self.assertRegex(content, r"(?ms)inlet\s*\{.*?value\s+uniform\s+\(12 0 0\);.*?\}")
        self.assertEqual(["inlet_velocity"], modifications)

    def test_inlet_velocity_does_not_modify_u_outlet(self) -> None:
        content, modifications = apply_reference_modifications(
            u_reference_file(PITZ_DAILY_U_CONTENT),
            {"inlet_velocity": "10"},
        )

        self.assertRegex(content, r"(?ms)outlet\s*\{.*?type\s+zeroGradient;.*?\}")
        self.assertNotRegex(content, r"(?ms)outlet\s*\{.*?value\s+uniform")
        self.assertEqual(["inlet_velocity"], modifications)

    def test_outlet_pressure_modifies_p_outlet_value_without_vector_format(self) -> None:
        content, modifications = apply_reference_modifications(
            p_reference_file(),
            {"outlet_pressure": "101325"},
        )

        self.assertRegex(content, r"(?ms)outlet\s*\{.*?value\s+uniform\s+101325;.*?\}")
        self.assertNotIn("uniform (101325 0 0)", content)
        self.assertEqual(["outlet_pressure"], modifications)

    def test_outlet_pressure_does_not_modify_p_inlet(self) -> None:
        content, modifications = apply_reference_modifications(
            p_reference_file(),
            {"outlet_pressure": "101325"},
        )

        self.assertRegex(content, r"(?m)^\s*inlet\s*\{\s*type\s+zeroGradient;\s*\}")
        self.assertEqual(["outlet_pressure"], modifications)

    def test_pitz_daily_boundary_aliases_are_accepted(self) -> None:
        u_content, u_modifications = apply_reference_modifications(
            u_reference_file(PITZ_DAILY_U_CONTENT),
            {"inflowVelocity": "8"},
        )
        p_content, p_modifications = apply_reference_modifications(
            p_reference_file(),
            {"pressureOutlet": "50"},
        )

        self.assertRegex(u_content, r"(?ms)inlet\s*\{.*?value\s+uniform\s+\(8 0 0\);.*?\}")
        self.assertRegex(p_content, r"(?ms)outlet\s*\{.*?value\s+uniform\s+50;.*?\}")
        self.assertEqual(["inlet_velocity"], u_modifications)
        self.assertEqual(["outlet_pressure"], p_modifications)

    def test_boundary_capability_skips_missing_patch_or_value(self) -> None:
        missing_patch_content, missing_patch_modifications = apply_reference_modifications(
            u_reference_file(PITZ_DAILY_U_WITHOUT_INLET_CONTENT),
            {"inlet_velocity": "10"},
        )
        missing_value_content, missing_value_modifications = apply_reference_modifications(
            u_reference_file(PITZ_DAILY_U_INLET_WITHOUT_VALUE_CONTENT),
            {"inlet_velocity": "10"},
        )

        self.assertEqual(PITZ_DAILY_U_WITHOUT_INLET_CONTENT, missing_patch_content)
        self.assertEqual([], missing_patch_modifications)
        self.assertEqual(PITZ_DAILY_U_INLET_WITHOUT_VALUE_CONTENT, missing_value_content)
        self.assertEqual([], missing_value_modifications)

    def test_pitz_daily_boundary_capabilities_are_case_name_scoped(self) -> None:
        simple_pitz_daily = ReferenceCase(
            case_name="pitzDaily",
            case_domain="incompressible",
            case_category="None",
            case_solver="simpleFoam",
        )
        pimple_pitz_daily = ReferenceCase(
            case_name="pitzDaily",
            case_domain="incompressible",
            case_category="RAS",
            case_solver="pimpleFoam",
        )
        cavity = ReferenceCase(
            case_name="cavity",
            case_domain="incompressible",
            case_category="cavity",
            case_solver="icoFoam",
        )
        other_simple_case = ReferenceCase(
            case_name="motorBike",
            case_domain="incompressible",
            case_category="None",
            case_solver="simpleFoam",
        )

        for reference_case in [simple_pitz_daily, pimple_pitz_daily]:
            supported = supported_change_keys(reference_case)
            self.assertIn("inlet_velocity", supported)
            self.assertIn("outlet_pressure", supported)

        for reference_case in [cavity, other_simple_case]:
            supported = supported_change_keys(reference_case)
            self.assertNotIn("inlet_velocity", supported)
            self.assertNotIn("outlet_pressure", supported)


if __name__ == "__main__":
    unittest.main()
