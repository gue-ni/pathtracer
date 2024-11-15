#!/bin/bash
set -e
set -x

timestamp=$(date "+%s")
git_hash=$(git rev-parse --short HEAD)

pt=./build/pt
config=$1

if [ "$#" -gt 1 ]; then
	samples=$2
else
	samples=8
fi

bounces=7
batch=10

result_path="artefacts/render_${git_hash}_${samples}_${bounces}_${datestamp}.png"

$pt $config $result_path $samples $bounces $batch


