#!/usr/bin/env bash
set -euo pipefail

log() { echo "[$(date --iso-8601=s)] $*" >&2; }

has_source_control() {
    if ! type -P git &>/dev/null; then
        return 1
    elif git status &>/dev/null ; then
        return 0
    fi
    return 1
}

get_revision_id() {
    # Use GitLab's CI data when present.
    git rev-parse HEAD
}

if ! has_source_control; then
    log "WARN: cannot resolve source control data"
    cat <<EOF
-DSOURCE_CONTROL_REVISION=\"na\"
-DSOURCE_CONTROL_AUTHOR=\"na\"
-DSOURCE_CONTROL_SAFE_SUBJECT=\"na\"
-DSOURCE_CONTROL_DATE=\"na\"
EOF
    exit 0
fi

log "INFO: resolving source control data"
git log -1 --pretty=format:'
-DSOURCE_CONTROL_REVISION=\"%h\"
-DSOURCE_CONTROL_AUTHOR=\"%ae\"
-DSOURCE_CONTROL_SAFE_SUBJECT=\"%f\"
-DSOURCE_CONTROL_DATE=\"%aI\"
' $(get_revision_id)
