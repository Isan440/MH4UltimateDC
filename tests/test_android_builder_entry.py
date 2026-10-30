import importlib.util
from pathlib import Path
import unittest

MODULE_PATH = Path(__file__).parents[1] / "tools" / "android_builder" / "entry.py"
SPEC = importlib.util.spec_from_file_location("mh4u_android_builder_entry", MODULE_PATH)
ENTRY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(ENTRY)


class AndroidBuilderEntryTests(unittest.TestCase):
    def test_arguments_default_to_no_mods_and_android_target(self):
        args = ENTRY._build_arguments(
            Path("/private/input/update.cia"),
            Path("/private/work"),
            Path("/app/lib/arm64-v8a/libmh4u_clang.so"),
            1,
            "O0",
            None,
        )
        self.assertIn("--android", args)
        self.assertIn("--no-mods", args)
        self.assertEqual(args[args.index("--cc") + 1],
                         "/app/lib/arm64-v8a/libmh4u_clang.so")
        self.assertEqual(args[args.index("--jobs") + 1], "1")

    def test_mod_directory_is_forwarded(self):
        args = ENTRY._build_arguments(
            Path("/private/input/update.cia"),
            Path("/private/work"),
            Path("/app/lib/arm64-v8a/libmh4u_clang.so"),
            2,
            "O1",
            Path("/private/mods"),
        )
        self.assertEqual(args[args.index("--mods") + 1], "/private/mods")
        self.assertNotIn("--no-mods", args)

    def test_rejects_unsupported_optimization(self):
        with self.assertRaises(ENTRY.AndroidBuildError):
            ENTRY._build_arguments(Path("dump"), Path("out"), Path("clang"), 1, "O3", None)

    def test_rejects_unbounded_worker_count(self):
        with self.assertRaises(ENTRY.AndroidBuildError):
            ENTRY._build_arguments(Path("dump"), Path("out"), Path("clang"), 8, "O0", None)


if __name__ == "__main__":
    unittest.main()
