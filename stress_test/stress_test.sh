#!/opt/homebrew/bin/bash
cat words.txt | xargs -n1 -P5 -I{} curl localhost:8080/{}
