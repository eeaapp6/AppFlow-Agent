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

    def test_supported_geometry_uses_reference_when_reference_score_is_strong(self) -> None:
        intent = parse_case_intent("\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u7ba1\u9053\u6d41")
        route = select_generation_route(intent, reference_score=70)

        self.assertEqual("reference_modify", route.selected_mode)

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


if __name__ == "__main__":
    unittest.main()
