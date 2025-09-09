cyan='\[\e[38;2;133;233;248m\]'
blue='\[\e[38;2;110;162;247m\]'
tan='\[\e[38;2;242;225;209m\]'
spk='\[\e[1m\e[38;2;255;154;239m\]'
lpk='\[\e[1m\e[38;2;255;207;232m\]'
white='\[\e[0m\]'

neut='\[\e[38;2;230;230;250m\]'
# cerulean = "0492C2"
## yellows
# fdf96
# fdfdaf also nice
# ff8994
bpy='\[\e[1m\e[38;2;253;253;150m\]'
py='\[\e[38;2;253;253;150m\]'
pys='\[\e[1m\e[38;2;253;253;175m\]'
pantone='\[\e[38;2;242;240;161m\]'
paleyel='\[\e[38;2;245;245;220m\]'
chifyel='\[\e[38;2;255;250;205m\]'
pty='\[\e[38;2;251;236;193m\]'

# light lav
lv='\[\e[38;2;230;230;250m\]'
vlv='\[\e[38;2;180;160;255m\]' # good
elv='\[\e[38;2;191;119;246m\]'
rlv='\[\e[38;2;177;102;255m\]'
nlv='\[\e[38;2;177;102;255m\]'

pc='\[\e[38;2;240;128;128m\]'
pgr='\[\e[38;2;135;206;235m\]'
pdb='\[\e[38;2;176;224;230m\]'

cool='\[\e[1m\e[38;2;158;206;106\]'

[[ -e "$HOME/.dircolors" ]] && eval "$(dircolors -b "$HOME/.dircolors")"

function parse_git_dirty {
  [[ $(git status --porcelain 2> /dev/null) ]] && echo "*"
}

function parse_git_branch {
  git branch --no-color 2> /dev/null | sed -e '/^[^*]/d' -e "s/* \(.*\)/ (\1$(parse_git_dirty))/"
}

nontext="$lv"
hostuser="$lpk"
ppath="$pty"
gbranch="$vlv"

function check_last_exit_code() {
    local LAST_EXIT_CODE=$?
    if [[ $LAST_EXIT_CODE == 0 ]]; then
        echo -e "${nontext}[${hostuser}${LAST_EXIT_CODE}${nontext}]"
    else
        echo -e "${nontext}[\e[31m${LAST_EXIT_CODE}${nontext}]"
    fi
}

PS1="$nontext┌─$spk\t$nontext─[$hostuser\u$nontext@$hostuser\h$nontext:$ppath\w$nontext]$gbranch\$(parse_git_branch)$nontext \n└─╼ $white"
