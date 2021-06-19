#!/bin/bash -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REPO_NAME="handbag"
WORKSPACE="${DIR}/../../${REPO_NAME}"
cd ${WORKSPACE}

bazel test '//...'
