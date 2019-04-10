#!/bin/bash
# Script to illustrate that ush is working correctly and robustly

./ush << EOF > script-nq
pwd
       echo "I have the high ground!"
  fortune     
ls           -l
ls -l -a
EOF
