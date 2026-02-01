
if [[ -f /usr/share/bash-completion/bash_completion ]]; then
    . /usr/share/bash-completion/bash_completion
elif [[ -f /usr/share/bash-completion/compat/bash_completion ]]; then
    . /usr/share/bash-completion/compat/bash_completion
elif [[ -f /etc/profile.d/bash_completion.sh ]]; then
    . /etc/profile.d/bash_completion.sh
fi

if [[ ! -f ~/git-completion.bash ]]; then
    curl -s https://raw.githubusercontent.com/git/git/master/contrib/completion/git-completion.bash > ~/git-completion.bash
fi

[[ -f ~/git-completion.bash ]] && . ~/git-completion.bash

