#!/bin/bash

FILE="$HOME/git-completion.bash"

if [ ! -f $FILE ]; then
    wget https://raw.githubusercontent.com/git/git/master/contrib/completion/git-completion.bash
    mv git-completion.bash $HOME/
fi
echo 'source ~/git-completion.bash' >> ~/.bashrc

git config --global alias.co checkout
git config --global alias.br branch
git config --global alias.ci commit
git config --global alias.st status

git config --global alias.unstage 'reset HEAD --'
git config --global alias.last 'log -1 HEAD'

git config --global color.ui true

git config --global branch.autosetuprebase always

git config --global alias.laststat '!f() { git diff --stat $1@{1}..$1 --dirstat=cumulative,files; }; f'

git config --global alias.s stash
git config --global alias.sp stash pop
