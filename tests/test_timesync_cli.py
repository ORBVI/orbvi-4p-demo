#!/usr/bin/env python3
"""Validate CLI rejection before any UART, PPS or system-clock access."""
import subprocess
import sys
import unittest

BINARY = sys.argv.pop(1)

class TimesyncCliTests(unittest.TestCase):
    def test_help_documents_default_and_protocols(self):
        result = subprocess.run([BINARY, '--help'], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0)
        self.assertIn('--protocol AUTO|ZDA|RMC', result.stdout)
        self.assertIn('Default protocol: AUTO (ZDA and RMC)', result.stdout)

    def test_bad_or_missing_protocol_rejected_before_apply(self):
        for args in (['--apply', '--protocol', 'INVALID'], ['--protocol', 'zda'],
                     ['--protocol'], ['--protocol', '--apply']):
            with self.subTest(args=args):
                result = subprocess.run([BINARY, *args], capture_output=True, text=True)
                self.assertEqual(result.returncode, 2)
                self.assertIn('Usage:', result.stderr)
                self.assertNotIn('Refusing --apply', result.stderr)

    def test_version(self):
        result = subprocess.run([BINARY, '--version'], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stdout, 'timesync_demo 0.1.1\n')

if __name__ == '__main__':
    unittest.main()
