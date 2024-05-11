#!/bin/bash

a="$1" #calea catre fisierul malitios
b="$2" #numele directorului in care vom muta
chmod 777 "$a"
cd $(dirname "a")
if test grep -q "corrupted" "$a" || grep -q "dangerous" "$a" || grep -q "risk" "$a" || grep -q "attack" "$a" || grep -q "malware" "$a" || grep -q "malicious" "$a" || grep -P -q '[^\x00-\x7F]' "$a"; then
    echo "Fisierul cu calea $a este periculos"
    mv "$(basename "$a")" "$b/"
    echo "Fisierul cu calea $a a fost mutat in $b"
fi
cd ..
