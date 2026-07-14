import unittest

from scripts.simagent_core.router import parse_case_intent, select_generation_route


class RouterPolicyTests(unittest.TestCase):
    def test_plain_pipe_flow_does_not_force_generated_rect_channel(self) -> None:
        intent = parse_case_intent("\u751f\u6210\u4e00\u4e2a\u7ba1\u9053\u6d41")
        route = select_generation_route(intent)

        self.assertEqual("reference_modify", route.selected_mode)
        self.assertNotEqual("rect_channel", route.geometry_type)

    def test_2d_pipe_flow_routes_to_generated_rect_channel(self) -> None:
        intent = parse_case_intent("\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u7ba1\u9053\u6d41")
        route = select_generation_route(intent)

        self.assertEqual("generated_case", route.selected_mode)
        self.assertEqual("rect_channel", route.geometry_type)
        self.assertGreaterEqual(route.confidence, 0.75)

    def test_supported_geometry_creation_overrides_strong_reference(self) -> None:
        intent = parse_case_intent("\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u7ba1\u9053\u6d41")
        route = select_generation_route(intent, reference_score=100)

        self.assertEqual("create", intent.action)
        self.assertTrue(intent.explicit_new_geometry)
        self.assertEqual("generated_case", route.selected_mode)
        self.assertEqual("rect_channel", route.geometry_type)

    def test_supported_geometry_generates_when_reference_score_is_weak(self) -> None:
        intent = parse_case_intent("\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u7ba1\u9053\u6d41")
        route = select_generation_route(intent, reference_score=69)

        self.assertEqual("generated_case", route.selected_mode)
        self.assertEqual("rect_channel", route.geometry_type)

    def test_explicit_new_geometry_overrides_strong_reference_score(self) -> None:
        intent = parse_case_intent(
            "\u6570\u636e\u5e93\u6ca1\u6709\u4e5f\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u7ba1\u9053"
        )
        route = select_generation_route(intent, reference_score=100)

        self.assertEqual("generated_case", route.selected_mode)
        self.assertEqual("rect_channel", route.geometry_type)

    def test_unknown_explicit_new_geometry_is_unsupported(self) -> None:
        intent = parse_case_intent("\u6570\u636e\u5e93\u6ca1\u6709\u4e5f\u751f\u6210\u4e00\u4e2a\u65b0\u51e0\u4f55")
        route = select_generation_route(intent)

        self.assertEqual("unsupported", route.selected_mode)
        self.assertIn("not supported", route.reason)

    def test_chinese_create_rectangular_pipe_routes_to_generated_case(self) -> None:
        intent = parse_case_intent("\u521b\u5efa\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u7ba1\u9053\uff0c\u957f\u5ea65m\uff0c\u9ad8\u5ea61m")
        route = select_generation_route(intent, reference_score=100)

        self.assertEqual("create", intent.action)
        self.assertTrue(intent.explicit_new_geometry)
        self.assertEqual("generated_case", route.selected_mode)

    def test_chinese_new_rectangular_channel_routes_to_generated_case(self) -> None:
        intent = parse_case_intent("\u65b0\u5efa\u4e00\u4e2a\u77e9\u5f62\u901a\u9053\u7b97\u4f8b")

        self.assertEqual("generated_case", select_generation_route(intent).selected_mode)
        self.assertTrue(intent.explicit_new_geometry)

    def test_english_generate_rectangular_channel_routes_to_generated_case(self) -> None:
        intent = parse_case_intent("Generate a 2D rectangular channel with length 5 and height 1")

        self.assertEqual("create", intent.action)
        self.assertEqual("rect_channel", intent.geometry_type)
        self.assertEqual("generated_case", select_generation_route(intent, reference_score=100).selected_mode)

    def test_explicit_reference_modification_stays_reference_modify(self) -> None:
        intent = parse_case_intent("\u4fee\u6539 airFoil2D \u6848\u4f8b\u7684 end time")
        route = select_generation_route(intent, reference_score=100)

        self.assertEqual("modify", intent.action)
        self.assertEqual("airFoil2D", intent.reference_case)
        self.assertFalse(intent.explicit_new_geometry)
        self.assertEqual("reference_modify", route.selected_mode)

    def test_explicit_modify_overrides_detected_rect_channel_geometry(self) -> None:
        intent = parse_case_intent("modify the rectangular channel case end time")

        self.assertEqual("modify", intent.action)
        self.assertEqual("rect_channel", intent.geometry_type)
        self.assertEqual("reference_modify", select_generation_route(intent).selected_mode)

    def test_explicit_reference_copy_stays_reference_copy(self) -> None:
        intent = parse_case_intent("\u590d\u5236 cavity tutorial")
        route = select_generation_route(intent, reference_score=100)

        self.assertEqual("copy", intent.action)
        self.assertEqual("cavity", intent.reference_case)
        self.assertEqual("reference_copy", route.selected_mode)

    def test_solver_qualified_existing_case_is_reference_copy(self) -> None:
        intent = parse_case_intent("create an icoFoam cavity case")

        self.assertEqual("copy", intent.action)
        self.assertEqual("cavity", intent.reference_case)
        self.assertEqual("reference_copy", select_generation_route(intent).selected_mode)

    def test_unsupported_created_geometry_does_not_fall_back_to_reference(self) -> None:
        intent = parse_case_intent("\u751f\u6210\u4e00\u4e2a\u4e09\u7ef4\u6da1\u8f6e\u53f6\u7247\u6d41\u573a")
        route = select_generation_route(intent, reference_score=100)

        self.assertEqual("create", intent.action)
        self.assertNotEqual("rect_channel", intent.geometry_type)
        self.assertEqual("unsupported", route.selected_mode)

    def test_result_file_generation_is_not_new_geometry_creation(self) -> None:
        intent = parse_case_intent("\u4f7f\u7528 airFoil2D \u751f\u6210\u65b0\u7684\u7ed3\u679c\u6587\u4ef6")
        route = select_generation_route(intent, reference_score=100)

        self.assertEqual("copy", intent.action)
        self.assertFalse(intent.explicit_new_geometry)
        self.assertNotEqual("generated_case", route.selected_mode)


if __name__ == "__main__":
    unittest.main()
