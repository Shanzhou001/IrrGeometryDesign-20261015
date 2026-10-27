"""Optional Git context; source and executable hashes remain the run identity."""
from pathlib import Path
import subprocess


def git_metadata(root):
    root=Path(root).resolve()
    result=dict(git_commit=None,git_working_tree_dirty=None,git_metadata_status='unavailable')
    def query(*args):
        return subprocess.check_output(['git',*args],cwd=root,text=True,stderr=subprocess.DEVNULL,
                                       creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0)).strip()
    try:
        top=Path(query('rev-parse','--show-toplevel')).resolve()
        if top!=root:
            result['git_metadata_status']='not_project_repository'
            return result
        result.update(git_commit=query('rev-parse','HEAD'),
                      git_working_tree_dirty=bool(query('status','--porcelain')),
                      git_metadata_status='available')
    except FileNotFoundError:
        result['git_metadata_status']='git_not_installed'
    except (OSError,subprocess.CalledProcessError):
        result['git_metadata_status']='not_available_for_this_source_copy'
    return result
