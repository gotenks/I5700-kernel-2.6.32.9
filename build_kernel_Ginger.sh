#!/bin/bash

. ./scripts/prepare_Ginger.sh prepare
./scripts/build_kernel.sh r880tom3q
. ./scripts/prepare_Ginger.sh cleanup
