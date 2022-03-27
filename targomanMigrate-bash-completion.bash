#/usr/bin/env bash

_completions()
{
    COMPREPLY=($(compgen -W "now tomorrow never" "${COMP_WORDS[1]}"))
}

complete -F _completions MigrationTool
