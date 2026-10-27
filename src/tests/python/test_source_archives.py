"""An exported source copy must run without a .git directory or Git executable."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from research.common.versioning import git_metadata
from research.experiments import audit_joint_studies as study


class SourceArchiveTests(unittest.TestCase):
    def test_missing_git_command_and_non_repository_are_metadata_only(self):
        with tempfile.TemporaryDirectory() as folder:
            for failure,status in [(FileNotFoundError('git'),'git_not_installed'),
                    (subprocess.CalledProcessError(128,['git']),'not_available_for_this_source_copy')]:
                with patch('research.common.versioning.subprocess.check_output',side_effect=failure):
                    result=git_metadata(folder)
                self.assertIsNone(result['git_commit']);self.assertEqual(result['git_metadata_status'],status)

    def test_parent_repository_is_not_mistaken_for_exported_project(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)/'export';root.mkdir()
            with patch('research.common.versioning.subprocess.check_output',return_value=folder):
                result=git_metadata(root)
            self.assertIsNone(result['git_commit']);self.assertEqual(result['git_metadata_status'],'not_project_repository')

    def test_recorder_completes_without_git_and_keeps_file_identity(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);source=root/'src/native/core.cpp';source.parent.mkdir(parents=True)
            source.write_text('int core() { return 1; }\n');exe=root/'native.exe';exe.write_bytes(b'fixture program')
            with patch.object(study,'ROOT',root),patch.object(study,'source_files',return_value=[source]),\
                 patch('research.common.versioning.subprocess.check_output',side_effect=FileNotFoundError('git')):
                recorder=study.Recorder(root/'run',exe,{},protocol='source_archive_test')
                recorder.finish()
            state=json.loads((root/'run/manifest.json').read_text())
            self.assertTrue(state['completed']);self.assertIsNone(state['git_commit'])
            self.assertEqual(state['git_metadata_status'],'git_not_installed')
            self.assertEqual(len(state['executable_sha256']),64)
            self.assertEqual(len(state['source_sha256']),1)


if __name__=='__main__':unittest.main()
