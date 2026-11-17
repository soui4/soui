"""Publishing/checking generated resource output must not hide stale artifacts."""
import tempfile
import unittest
from pathlib import Path

from build_resources import sync_outputs


class ResourcePublishingTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.target = Path(temporary.name) / "resource.h"

    def test_timestamp_only_changes_are_not_published(self):
        previous = b"\xef\xbb\xbf//stamp:abcd\r\n#define ID 7\r\n"
        self.target.write_bytes(previous)
        self.assertEqual(sync_outputs([self.target], [previous.replace(b"abcd", b"1234")]), [])
        self.assertEqual(self.target.read_bytes(), previous)

    def test_check_detects_semantic_changes_without_writing(self):
        self.target.write_bytes(b"#define ID 7\n")
        changed = b"#define ID 8\n"
        self.assertEqual(sync_outputs([self.target], [changed], check=True), ["resource.h"])
        self.assertEqual(self.target.read_bytes(), b"#define ID 7\n")
        self.assertEqual(sync_outputs([self.target], [changed]), ["resource.h"])
        self.assertEqual(self.target.read_bytes(), changed)

    def test_check_does_not_create_a_missing_output(self):
        self.assertEqual(sync_outputs([self.target], [b"new"], check=True), ["resource.h"])
        self.assertFalse(self.target.exists())


if __name__ == "__main__":
    unittest.main()
