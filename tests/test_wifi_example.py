#!/usr/bin/env python3
"""Host-only Wi-Fi lifecycle tests; all network commands are mocked."""
import fcntl
import os
from pathlib import Path
import pty
import signal
import subprocess
import tempfile
import termios
import time
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/wifi_example.sh'


class WifiExample(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='wifi-test-')
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        bin_dir = self.root / 'bin'
        bin_dir.mkdir()
        (self.root / 'state').mkdir()
        self.log = self.root / 'calls'
        self.log.write_text('')
        self.env = dict(os.environ, PATH=f'{bin_dir}:{os.environ["PATH"]}',
                        TMPDIR=str(self.root), WIFI_STATE_ROOT=str(self.root / 'state'),
                        TEST_ROOT=str(self.root))
        mocks = {
            'id': '#!/bin/sh\necho 0\n',
            'ip': '''#!/bin/sh
printf 'ip %s\\n' "$*" >> "$TEST_ROOT/calls"
case "$*" in
  'link show dev test0') echo '2: test0: <BROADCAST,MULTICAST> mtu 1500';;
  '-o link show dev test0')
    if [ -e "$TEST_ROOT/up" ]; then echo '2: test0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500';
    else echo '2: test0: <BROADCAST,MULTICAST> mtu 1500'; fi;;
  'link set dev test0 up') touch "$TEST_ROOT/up";;
  'link set dev test0 down') rm -f "$TEST_ROOT/up";;
  'addr show dev test0') echo 'mock address';;
  '-o addr show dev test0') [ "${TEST_FAIL_ADDR:-0}" = 0 ] || exit 1;;
  '-o -4 addr show dev test0') [ ! -e "$TEST_ROOT/lease" ] || echo 'inet 192.0.2.10/24';;
  '-4 addr replace 192.0.2.10/255.255.255.0 dev test0') touch "$TEST_ROOT/lease";;
  '-4 addr flush dev test0') rm -f "$TEST_ROOT/lease";;
esac
''',
            'iw': '''#!/bin/sh
printf 'iw %s\\n' "$*" >> "$TEST_ROOT/calls"
case "$*" in
 'dev test0 info') echo 'Interface test0';;
 'dev test0 link') echo 'Not connected.';;
 'dev test0 scan') printf '\\tSSID: Sample\\n\\tSSID: Sample\\n\\tSSID: TestNet\\n';;
esac
''',
            'wpa_supplicant': '#!/bin/sh\nexec sleep 60\n',
            'wpa_cli': '#!/bin/sh\necho wpa_state=COMPLETED\n',
            'udhcpc': '''#!/bin/sh
printf 'udhcpc %s\n' "$*" >> "$TEST_ROOT/calls"
script=
while [ "$#" -gt 0 ]; do
  if [ "$1" = -s ]; then script=$2; shift 2; else shift; fi
done
[ -n "$script" ] || exit 3
[ "${TEST_DHCP_EXIT:-0}" = 0 ] || exit 4
interface=test0 ip=192.0.2.10 subnet=255.255.255.0 "$script" bound || exit 5
exec sleep 60
''',
            'hostapd': '#!/bin/sh\nexec sleep 60\n',
            'dnsmasq': '#!/bin/sh\nexec sleep 60\n',
        }
        for name, contents in mocks.items():
            path = bin_dir / name
            path.write_text(contents)
            path.chmod(0o755)
        self.proc = None
        self.addCleanup(self.stop_proc)

    def run_script(self, *args):
        return subprocess.run([str(SCRIPT), *args], env=self.env, capture_output=True,
                              text=True, timeout=5)

    def wait_state(self):
        state = self.root / 'state' / 'orbvi-wifi-test0.state'
        for _ in range(100):
            if state.exists():
                return state
            time.sleep(.05)
        self.fail('no managed session was registered')

    def stop_proc(self):
        if self.proc is not None and self.proc.poll() is None:
            self.proc.terminate()
            self.proc.communicate(timeout=5)

    def test_status_scan_refuse_active(self):
        self.assertEqual(self.run_script('--status', 'test0').returncode, 0)
        scan = self.run_script('scan', 'test0')
        self.assertEqual(scan.returncode, 0, scan.stderr)
        self.assertEqual(scan.stdout.strip().splitlines(), ['Sample', 'TestNet'])
        self.assertFalse((self.root / 'up').exists())
        (self.root / 'up').touch()
        self.assertNotEqual(self.run_script('scan', 'test0').returncode, 0)
        self.assertNotEqual(self.run_script('sta', 'test0', 'TestNet', '--open').returncode, 0)
        self.assertFalse(list((self.root / 'state').iterdir()))
        (self.root / 'up').unlink()
        self.env['TEST_FAIL_ADDR'] = '1'
        self.assertNotEqual(self.run_script('scan', 'test0').returncode, 0)
        self.assertFalse((self.root / 'up').exists())

    def test_open_sta_and_scoped_stop(self):
        self.proc = subprocess.Popen([str(SCRIPT), 'sta', 'test0', 'TestNet', '--open'],
                                     env=self.env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        state = self.wait_state()
        for _ in range(100):
            if (self.root / 'up').exists():
                break
            time.sleep(.05)
        dirs = list(self.root.glob('orbvi-wifi.*'))
        self.assertEqual(len(dirs), 1)
        for _ in range(100):
            if (self.root / 'lease').exists():
                break
            time.sleep(.05)
        self.assertTrue((self.root / 'lease').exists(), 'mock DHCP hook did not assign an address')
        config = dirs[0] / 'wpa.conf'
        self.assertIn('key_mgmt=NONE', config.read_text())
        self.assertEqual(config.stat().st_mode & 0o777, 0o600)
        self.assertEqual(self.run_script('stop', 'test0').returncode, 0)
        self.proc.communicate(timeout=5)
        self.assertFalse(state.exists())
        self.assertFalse(dirs[0].exists())
        self.assertFalse((self.root / 'up').exists())
        self.assertIn('ip -4 addr flush dev test0', self.log.read_text())
        self.assertIn('ip -4 addr replace 192.0.2.10/255.255.255.0 dev test0', self.log.read_text())
        self.assertIn('udhcpc -i test0 -f -n -t 5 -T 3 -s ', self.log.read_text())
        self.assertNotIn('route ', self.log.read_text())
        self.assertFalse((self.root / 'lease').exists())
        self.assertNotIn('eth0', self.log.read_text())
        self.assertNotEqual(self.run_script('stop', 'test0').returncode, 0)

    def test_sta_dhcp_failure_cleans_up(self):
        self.env['TEST_DHCP_EXIT'] = '1'
        result = self.run_script('sta', 'test0', 'TestNet', '--open')
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((self.root / 'up').exists())
        self.assertFalse(list((self.root / 'state').iterdir()))
        self.assertFalse(list(self.root.glob('orbvi-wifi.*')))

    def test_sta_secret_escaping_and_no_output(self):
        master, slave = pty.openpty()
        def tty_session():
            os.setsid()
            fcntl.ioctl(slave, termios.TIOCSCTTY, 0)
        self.proc = subprocess.Popen([str(SCRIPT), 'sta', 'test0', 'SafeWifi'],
                                     env=self.env, stdin=slave, stdout=slave, stderr=slave,
                                     preexec_fn=tty_session)
        os.close(slave)
        try:
            prompt = b''
            while b'Wi-Fi password: ' not in prompt:
                prompt += os.read(master, 1024)
            os.write(master, b'safe\\"pass99\n')
            state = self.wait_state()
            config = list(self.root.glob('orbvi-wifi.*/wpa.conf'))[0]
            self.assertIn('psk="safe\\\\\\"pass99"', config.read_text())
            self.assertEqual(config.stat().st_mode & 0o777, 0o600)
            self.assertEqual(self.run_script('stop', 'test0').returncode, 0)
            self.proc.wait(timeout=5)
            self.assertFalse(state.exists())
            output = b''
            try:
                while True:
                    output += os.read(master, 1024)
            except OSError:
                pass
            self.assertNotIn(b'pass99', output)
        finally:
            os.close(master)

    def test_ap_secret_and_validation(self):
        for args in [('ap', 'test0', 'AP', '192.168.5.1', '192.168.5.1', '192.168.5.10', '1'),
                     ('ap', 'test0', 'AP', '192.168.5.1', '192.168.5.2', '192.168.6.3', '1'),
                     ('ap', 'test0', 'AP', '192.168.5.1', '192.168.5.2', '192.168.5.10', '14'),
                     ('sta', 'test0', 'bad\nssid', '--open')]:
            self.assertNotEqual(self.run_script(*args).returncode, 0)
        master, slave = pty.openpty()
        def tty_session():
            os.setsid()
            fcntl.ioctl(slave, termios.TIOCSCTTY, 0)
        self.proc = subprocess.Popen([str(SCRIPT), 'ap', 'test0', 'SafeAP', '192.168.5.1',
                                      '192.168.5.2', '192.168.5.20', '1'],
                                     env=self.env, stdin=slave, stdout=slave, stderr=slave,
                                     preexec_fn=tty_session)
        os.close(slave)
        try:
            prompt = b''
            for _ in range(100):
                prompt += os.read(master, 1024)
                if b'Wi-Fi password: ' in prompt:
                    break
            self.assertIn(b'Wi-Fi password: ', prompt)
            os.write(master, b'sensitive123\n')
            state = self.wait_state()
            config = list(self.root.glob('orbvi-wifi.*/hostapd.conf'))[0]
            self.assertIn('wpa_passphrase=sensitive123', config.read_text())
            self.assertEqual(config.stat().st_mode & 0o777, 0o600)
            self.assertNotEqual(self.run_script('ap', 'test0', 'Other', '192.168.5.1',
                                               '192.168.5.2', '192.168.5.20', '1').returncode, 0)
            self.assertEqual(self.run_script('stop', 'test0').returncode, 0)
            self.proc.wait(timeout=5)
            self.assertFalse(state.exists())
            self.assertFalse(config.exists())
            output = b''
            try:
                while True:
                    output += os.read(master, 1024)
            except OSError:
                pass
            self.assertNotIn(b'sensitive123', output)
            self.assertIn('ip addr del 192.168.5.1/24 dev test0', self.log.read_text())
        finally:
            os.close(master)


if __name__ == '__main__':
    unittest.main()
