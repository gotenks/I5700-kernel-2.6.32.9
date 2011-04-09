#!/bin/bash

. ./scripts/prepare_Froyo.sh prepare
./scripts/build_kernel.sh r880
. ./scripts/prepare_Froyo.sh cleanup
