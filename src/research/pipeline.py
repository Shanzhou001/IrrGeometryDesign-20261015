"""Build, run, test and browse the project."""
import argparse
import os
import subprocess
import sys

from research.common.paths import ROOT, WORKSPACE, executable
from research.common.processes import native_environment


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    build = commands.add_parser('build', help='build the native application')
    build.add_argument('--mode', choices=['Release', 'Debug'], default='Release')
    run = commands.add_parser('run', help='run the GUI or native CLI')
    run.add_argument('--mode', choices=['Release', 'Debug'], default='Release')
    run.add_argument('arguments', nargs=argparse.REMAINDER)
    commands.add_parser('test', help='run Python regression tests; external data tests may skip')
    check = commands.add_parser('check', help='record native and Python checks; use --full for external-data integration')
    check.add_argument('arguments', nargs=argparse.REMAINDER)
    viking = commands.add_parser('viking', help='validate or run the registered small Viking field-data study')
    viking.add_argument('arguments', nargs=argparse.REMAINDER)
    viking_v2 = commands.add_parser('viking-v2', help='run pilot-assisted recovery, coverage and multi-window/hidden-set experiments')
    viking_v2.add_argument('arguments', nargs=argparse.REMAINDER)
    commands.add_parser('figures', help='refresh the image gallery without rerunning experiments')
    module = commands.add_parser('module', help='run a research module with its own arguments')
    module.add_argument('name', help='for example research.validation.audit_numerics')
    module.add_argument('arguments', nargs=argparse.REMAINDER)
    arguments=sys.argv[1:]
    if arguments[:1] and arguments[0] in ('check','viking','viking-v2'):
        target = {'check':'research.validation.run_audit_checks', 'viking':'research.experiments.viking_study',
                  'viking-v2':'research.experiments.viking_v2'}[arguments[0]]
        forwarded=arguments[1:]
        if forwarded[:1]==['--']:forwarded=forwarded[1:]
        arguments=['module',target,*forwarded]
    args = parser.parse_args(arguments)

    if args.command == 'figures':
        from research.analysis.gallery import render
        render()
        return

    WORKSPACE.mkdir(parents=True, exist_ok=True)
    env = native_environment(render=args.command == 'run')
    env['IRR_WORKSPACE'] = str(WORKSPACE)
    env['PYTHONPATH'] = str(ROOT/'src') + os.pathsep + env.get('PYTHONPATH', '')
    env['PYTHONDONTWRITEBYTECODE'] = '1'
    if args.command == 'build':
        command = ['cmd', '/c', str(ROOT/'build.bat'), args.mode]
    elif args.command == 'run':
        program = executable(args.mode)
        if not program.is_file():
            parser.error('application is not built; run pipeline.py build first')
        arguments = args.arguments[1:] if args.arguments[:1] == ['--'] else args.arguments
        command = [str(program), *arguments]
    elif args.command == 'test':
        command = [sys.executable, '-B', '-m', 'unittest', 'discover',
                   '-s', str(ROOT/'src/tests/python'), '-t', str(ROOT/'src'), '-v']
    else:
        command = [sys.executable, '-B', '-m', args.name, *args.arguments]
    raise SystemExit(subprocess.call(command, cwd=WORKSPACE, env=env))


if __name__ == '__main__':
    main()
