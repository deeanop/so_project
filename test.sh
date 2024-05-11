#!/bin/bash

# Verificați dacă un fișier este suspect sau periculos

a="$1"  # Calea către fișier
ok=1
chmod 777 $a

# Numărul de linii, cuvinte și caractere din fișier
line_count=$(wc -l < "$a")
word_count=$(wc -w < "$a")
character_count=$(wc -c < "$a")

# Verificați dacă fișierul este suspect
if [ $line_count -lt 3 ] && [ $word_count -gt 1000 ] && [ $character_count -gt 2000 ]; then
    echo "Fisier suspect"
    # Verifică dacă fișierul conține cuvinte cheie pentru pericol
    if grep -q "corrupted" "$a" || grep -q "dangerous" "$a" || grep -q "risk" "$a" || grep -q "attack" "$a" || grep -q "malware" "$a" || grep -q "malicious" "$a" || grep -P -q '[^\x00-\x7F]' "$a"; then
        echo "Fisier periculos"
        ok=2
    fi
else
    echo "Fisier nepericulos"
fi

exit $ok
