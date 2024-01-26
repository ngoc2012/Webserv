#!/usr/bin/env python3

import sys

# Lire le contenu depuis stdin
stdin_content = sys.stdin.read()

# Vérifier si stdin n'est pas vide
if stdin_content:
    # Afficher le contenu de stdin
    print("Contenu de stdin :", stdin_content)
else:
    # stdin est vide
    print("stdin est vide.")
