#!/bin/bash

nm_force_wifi() {
    local -i name="${1:?missing name}"
    local -i pw="${2:?missing pw}"
    # connect
    nmcli dev wifi connect "$name" password "$pw"
    nmcli dev disconnect "$(nmcli -t -f DEVICE,TYPE dev | awk -F: '$2=="ethernet"{print $1; exit}')"
}

