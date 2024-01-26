<?php
// Lire le contenu du corps de la requête POST
$body = file_get_contents("php://input");

// Vérifier si le corps n'est pas vide
if (!empty($body)) {
    // Afficher le contenu du corps de la requête
    echo "Contenu du corps de la requête : " . $body;
} else {
    // Le corps de la requête est vide
    echo "Le corps de la requête est vide.";
}
?>


