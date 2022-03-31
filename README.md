# c_project
pour le module de systemes et réseaux

HOW TO RUN

-cd PROJET/
-make Makefile compile
-./server <"n° port">
To join the chat server on any other terminal, you can do:
-./client <"n° port>

HOW TO USE GIT WITH VSCODE AND CLI

on vscode, at your left, you have a vertical tool bar. Select the third option. Here you'll see all your git changes.
to stage a file, just click on the "+". then once all your files are staged, you can write a commit message, and click
on validate, above the commit message input (it corresponds to a git commit).
Then type into the terminal "git push".

GIT TIPS
-if you have commited something without pushing it, but forgot to add something in one of your files, you can just do your
modifications, add stage it in vscode and type in the terminal (git commit --amend)
-if you wanna stage your work, it's just "git stash", then if you want it back, its "git stash apply"
-if you wanna pull from the repo, you can type "git pull --rebase"

TO REMOVE BEFORE RELEASE

for this project, will do it similarly to what this guy has done: 
https://github.com/nikhilroxtomar/Chatroom-in-C/

as for now, the server part is working, the client one, not yet.