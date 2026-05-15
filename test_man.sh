#!/bin/bash
# ====================== ==============================================================================
cd bin

# ==============================================================================
# MODULE 1 : FICHIERS ET PERMISSIONS (my_attr, my_chmod, my_chown, my_chgrp, my_unlink)
# ==============================================================================

# 1. Créer un fichier de test vide
touch test_file.txt

# 2. Vérifier les attributs initiaux du fichier
./my_attr test_file.txt

# 3. Ajouter la permission d'exécution (x) pour l'utilisateur (u)
./my_chmod test_file.txt u x

# 4. Vérifier que le 'x' a bien été ajouté au "Mode"
./my_attr test_file.txt

# 5. Changer le groupe
sudo ./my_chgrp root test_file.txt

# 6. Changer le propriétaire
sudo ./my_chown root test_file.txt

# 7. Vérifier les nouveaux propriétaires
./my_attr test_file.txt

# 8. Supprimer le fichier
sudo ./my_unlink test_file.txt

# 9. Prouver que le fichier a disparue 
./my_attr test_file.txt


# ==============================================================================
# MODULE 2 : REPERTOIRES (my_mkdir, my_rmdir, my_ls)
# ==============================================================================

# 1. Créer un nouveau répertoire avec les permissions 0755
./my_mkdir 0755 test_folder

touch test_folder/fichier1.txt
touch test_folder/fichier2.txt

# 3. Tester la commande ls classique
./my_ls test_folder

# 4. Créer un sous-dossier pour tester la récursivité
./my_mkdir 0755 test_folder/sub_folder
touch test_folder/sub_folder/fichier3.txt

# 5. Tester la commande ls récursive
./my_ls -r test_folder

# 6. Tenter de supprimer le dossier
./my_rmdir test_folder

# 7. Vider le dossier proprement pour tester rmdir
rm -r test_folder/*
./my_rmdir test_folder

# 8. Vérifier la suppression
./my_ls .


# ==============================================================================
# MODULE 3 : PROCESSUS (lance_app, track_apps)
# ==============================================================================

# 1
./lance_app date

# 2. Tester track_apps
./track_apps ls pwd whoami date
