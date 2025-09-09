#!/bin/bash

find "${1-.}" -type f -mtime +60 -ls
