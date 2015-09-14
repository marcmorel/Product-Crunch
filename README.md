Product Crunch
============

 ** Product Crunch ** permet de transformer un flux XML ou CSV de produits en un catalogue exploitable en HTML5.

 Ce catalogue comprend les fonctionnalités suivantes :
 
 * navigation par catégories, sous-catégories, etc ...
 * recherche par texte libre
 * recherche prédictive (résultats au fur et à mesure de la saisie)
 * gestion de champs propriétaires dans les fiches produits
 * gestion des déclinaisons (taille, couleurs ...) sur 2 niveaux
 * habillage différent en fonction de la catégorie affichée


Product Crunch se compose de deux modules, écrits en C++ et en Javascript. 

Le **module en C++** génère une arborescence de fichiers JSON à partir du fichier source et d'un fichier de paramétrage.

Le **module en javascript** est l'application HTML5 de présentation du catalogue. Il se base sur les fichiers JSON et ne requiert pas de base de données.

Les deux modules peuvent fonctionner indépendamment, le seul lien entre les deux étant le système de fichier JSON.

## Le module C++

### Dépendances C++


Les dépendances de Product Crunch sont :

#### BOOST

La librairie **BOOST** (package `libboost-dev` sur Debian). La version utilisée dans cette repository est  1.49 (package spécifique `libboost1.49-dev`). Cette librairie est utilisée pour les expressions régulières, la gestion des options de ligne de commande, les tables de hashage (unordered_map), la gestion du multithread. Cette librairie est linkée, elle n'est pas incluse dans les sources.


La commande suivante permet d'installer la version sur la machine cible :

```
apt-get install -y libboost-all-dev
```

#### PUGIXML

La librairie **pugixml-1.2**. Celle librairie est utilisée pour le parsing XML des fichiers Google Merchant. Pas d'installation à prévoir : les sources sont dans la repository.

Licence MIT : http://pugixml.org/docs/quickstart.html#license

#### JSON SPIRIT

La librairie **json-spirit** en version 4.06. Cette librairie est utilisée pour parser le fichier de paramétrage du crunch (rédigé en JSON) ainsi que pour créer les fichiers JSON constituant l'output du crunch. Pas d'installation à prévoir : les sources sont dans la repository.

Voir un article exemple d'utilisation :
http://www.codeproject.com/Articles/20027/JSON-Spirit-A-C-JSON-Parser-Generator-Implemented

Licence MIT : http://www.codeproject.com/Articles/20027/JSON-Spirit-A-C-JSON-Parser-Generator-Implemented

#### LIBCURL

La librairie **libcurl** est utilisée pour télécharger les images citées dans le flux google merchant.
Elle est linkée dans l'application.

La commande suivante permet d'installer la version sur la machine cible :

```
apt-get install -y libcurl4-openssl-dev
```


Licence dérivée de MIT/X : http://curl.haxx.se/docs/copyright.html

#### OPENSSL

La librairie **openSSL** est utilisée pour calculer les MD5.
Elle est linkée dans l'application.

La commande suivante permet d'installer la version sur la machine cible :

```
apt-get install -y openssl
```


Licence Apache : https://www.openssl.org/about/

#### ImageMagick

La librairie **imagemagick** est utilisée pour calculer des vignettes d'images. Elle s'installe avec le package **libmagick++-dev**.

La commande suivante permet d'installer la version sur la machine cible :

```
apt-get install -y libmagick++-dev
```

Licence Apache 2.0 : http://www.imagemagick.org/script/index.php

### Compilation


#### Options de compilation :

|option|description|
|---|---|
| all        | build bin/release/crunch     |
| debug      | build bin/debug/crunch       |
| clean      | clean release obj and binary |
| cleandebug | clean release obj and binary |
| cleanall   | cleandebug and clean         |
|help|affiche la liste des options de compilation|

Le crunch se compile à l'aide du makefile fourni dans la repository. `make clean` permet de nettoyer les fichiers intermédiaires et `make`ou `make all` compile projet.


Le projet fonctionne avec `g++`. Pas d'installation spécifique sur Debian.

Nous avons le binaire release dans bin/release/ et ces fichiers o dans build/release/.

Nous avons le binaire debug dans bin/debug/ et ces fichiers o dans build/debug/.

### Utilisation

Le crunch prend 1 paramètre :

* `config-file` : indique le fichier de paramétrage à utiliser. La syntaxe est décrite ci-après.

L'exécutable peut aussi être appelé avec `--help` comme paramètre pour repréciser cette liste de paramètres.


Paramétrage
-----------
Le paramétrage principal du crunch se fait dans fichier `source-file` qui est au format JSON.

*Note : tous les paths de répertoire doivent finir par un /*
 
Les clés de paramétrage sont les suivantes : 

| Clé  |  Format | Signification | Exemple |
| ---- | ------- | ------------- | ------- |
| **`directory`** | string / obligatoire | Path de destination du crunch généré | /content/orchestra/crunch/ |
| **`source-file`** | string / obligatoire | Fichier source du crunch. Si l'URL débute par http:// ou https://, un download aura lieu. Sinon on estime qu'il s'agit d'un fichier local. | https://example.com/example.xml |
| `diff-directory` | string / optionnel| Si on souhaite que le crunch calcule la différence (DIFF) par rapport au précédent crunch, on indique ici le path du crunch précédent | /content/orchestra/previous-crunch/ |
| `status-json-file` | string / optionnel | Si on calcule un DIFF, le résultat du DIFF sera écrit sous forme JSON dans le fichier `status-json-file` | /content/orchestra/crunchstatus.json |
| `server-directory` | string / optionnel | Si on souhaite garder une version serveur du crunch en plus de la version synchronisé sur tablettes, on indique le path de destination | /content/orchestra/server-crunch/ |
| `server-url` | string / optionnel | Dans le cas où on garde une version serveur, `server-url` sera l'URL d'accès à ce serveur | http://catalog.server.com/catalog/orcchestra/ |
| `source-password` | string / optionnel | Mot de passe pour un download protégé par HTACCESS | strongpassword1234 |
| `source-login` | string / optionnel | Login pour un download protégé par HTACCESS | user |
| `photo-hashkey` | integer / optionnel | Taille de la hashkey pour regrouper les ressources de photos. Si `-1` (valeur par défaut), cette valeur sera calculée automatiquement par le crunch | 2 |
| `file-hashkey` | integer / optionnel | Taille de la hashkey pour regrouper les informations produits par fichier. Si `-1` (valeur par défaut), cette valeur sera calculée automatiquement par le crunch | 2 |
| `resource-hashkey` | integer / optionnel | Taille de la hashkey pour regrouper les fichiers produits par répertoire. Si `-1` (valeur par défaut), cette valeur sera calculée automatiquement par le crunch | 2 |
| `search-hashkey` | integer / optionnel | Taille de la hashkey pour regrouper les informations d'index par fichier. Si `-1` (valeur par défaut), cette valeur sera calculée automatiquement par le crunch | 2 |
| `download-pictures` | boolean / optionnel | Détermine si les images doivent être téléchargées. Valeur par défaut `true` | false |
| `resize-pictures`| boolean / optionnel | Détermine si les images doivent être redimensionnées. Valeur par défaut `false` | true |
| `max-width`| integer / optionnel | Dans le cas d'un redimensionnement, donne la largeur max. | 300 |
| `max-height`| integer / optionnel | Dans le cas d'un redimensionnement, donne la hauteur max. | 500 |
| `create-thumbnail` | boolean / optionnel | Détermine si les vignettes doivent être générées. Valeur par défaut `false` | true |
| `thumbnail-width`| integer / optionnel | Dans le cas d'une génération de vignette, donne la largeur max. | 300 |
| `thumbnail-height`| integer / optionnel | Dans le cas d'une génération de vignette, donne la hauteur max. | 500 |
| `transmit-pictures` | boolean / optionnel | Détermine si les images doivent être transmises sur les devices. Valeur par défaut `true` | true |
| `transmit-thumbnail` | boolean / optionnel | Détermine si les vignettes doivent être transmises sur les devices. Valeur par défaut `false` | true |
| `transmit-catalog` | boolean / optionnel | Détermine si les fichers catalogues (informations produits et catégories) doivent être transmis sur les devices. Valeur par défaut `true` | true |
| `generate-product-file` | boolean / optionnel | Détermine si les fichiers *produits* doivent être générés. Valeur par défaut `true` | true |
| `generate-model-file` | boolean / optionnel | Détermine si les fichiers *models* doivent être générés. Valeur par défaut `true` | true |
| `generate-category-file` | boolean / optionnel | Détermine si les fichiers *category* doivent être générés. Valeur par défaut `true` | true |
| `category-separator` | string / optionnel | Définit le séparateur de catégories pour un produit. par exemple pour `"Vêtements > Homme"`, le séparateur est `>`. La valeur par défaut est `&gt;`| \\$SUB\\$ |
| `custom-model-fields` | array[string] / optionnel | Définit une liste de champs supplémentaire à insérer dans les informations `model` |[ "club_price", "club_text"] |
| `custom-product-fields` | array[string] / optionnel | Définit une liste de champs supplémentaire à insérer dans les informations `produit` |[ "club_price", "club_text"] |
| `custom-reference--fields` | array[string] / optionnel | Définit une liste de champs supplémentaire à insérer dans les informations `reference` |[ "club_price", "club_text"] |
| `fields-name-replacement` | object `{ "norm1":"specific1", ... ,  "normN":"specificN" }`/ optionnel | permet de substituer des labels du XML source qui ne seraient pas conforme à la norme Google Merchant. Dans l'exemple ci-contre, le champs indiquant les catégories ( `g:google_product_category` dans la norme) a été redéfini pour être trouvé dans le fichier source | `{ "g:google_product_category" : "categories", "g:item_group_id" : "g:mpn" }`|
| `fields-regexp` | object `{ "nom1":"regexp1" , ... , "nomN":"regexpN"}`/ optionnel | Définit des expressions régulières à appliquer aux champs. Récupère en résultat le premier groupe capturant |  `{ "g:item_group_id" : "^(.+)\\\\.+$", "g:price" : "^([0-9]*\\.?[0-9]+) EUR", "club_price" : "^([0-9]*\\.?[0-9]+) EUR" }` |
| `search` | array[ object ] / optionnel | Définit les index de recherches. Le format précis dépend du type d'index recherche; il est défini ci-après. | [ { "name":"progressive","key" :"title", "type" : "progressive","object":"model", "transmit":false},  { "name":"id","key" :"g:id", "type" :"exact","object":"product","transmit":true},{ "name":"ean","key" :"g:gtin", "type" : "exact","object":"product","transmit":true}, { "name":"mpn","key" :"g:mpn", "type" : "exact","object":"product","transmit":false}] |
   
     

Résultats du Product Crunch C++
-------------------
En fonction des paramètres du crunch, des fichiers différents sont produits


### Coté serveur

Si `server_directory` est défini, un crunch "version serveur" est généré :

|`download_ pictures`|`server_ directory`| `transmit_ pictures`|`create_ thumbnail`|`transmit_ thumbnail`|Images|Vignettes|
|-----------|-------------|---------------|-----------|------|-----------|---|
| `false`   | N/A | N/A | N/A | N/A | Non téléchargées. Les URLs des images sont absolues et restent sur le serveur client. Pas d'image sur les devices | Aucun calcul de vignette possible |
| `true`    | `undefined`| `false` | `false`| `false`| **Cas impossible** : les photos doivent être téléchargées mais non transmises sur les devices. Donc `server_ directory` devrait être défini.| Pas de génération de vignette |
| `true`    | `path défini`| `false` | `false`| `false`| Téléchargées. URL des images absolues, vers serveur server. Pas d'image sur les devices| Pas de génération de vignette |
| `true`    | `undefined`| `true` | `false`| `false`| Téléchargées et transmises sur les devices. Pas de stockage serveur| Pas de génération de vignette |
| `true`    | `undefined`| `true` | `true`| `false`| Téléchargées et transmises sur les devices. Pas de stockage serveur| **Cas impossible** Les vignettes sont générées, non transmises et pas stockables :  `server_ directory` devrait être défini. |
| `true`    | `defined`| `true` | `true`| `false`| Téléchargées et transmises sur les devices. Pas de stockage serveur|  Les vignettes sont générées, non transmises et stockées sur le serveur. Les vignettes étant de taille très inférieure aux images, cette confugration n'a que peu d'intéret. |
| `true`    | `defined`| `false` | `true`| `true`| Téléchargées. URL des images absolues, vers serveur server. Pas d'image sur les devices |  Les vignettes sont générées, stockées & transmises.Cela permet d'avoir des visuels disponibles en offline avec un volume faible. |
| `true`    | `defined`| `true` | `true`| `true`| Téléchargées et transmises.  |  Les vignettes sont générées, stockées & transmises. La génération des vignettes ne présente pas beaucoup pas d'intérêt car les images pleine taille sont transmises aussi. Utile lorsque l'application est lente et que le transfert de données ne pose pas de problème|


Architecture du module C++
---------------------------

Le namespace des classes de cet outil est `crunch` (sans majuscule)
La classe maitresse est `crunch::Crunch`.

Elle est instanciée par la fonction `main` en tant que variable locale appelée `crunchInstance`.

Pour gérer toutes les données de configuration (que ce soient des données par défaut ou des données issues du fichier de configuration), cette classe possède dans ses membres un objet de type `CrunchConfig` : c'est un singleton (`cunchInstance.crunchConfig`), dont la référence va être utilisée dans de très nombreux endroits du programme.


Deux méthodes de Crunch sont appelées par `main`:



### crunch::Crunch::init

Cette fonction est appelée avec un seul paramètre, le path absolu du fichier de configuration au format JSON.

Elle réalise les opération suivantes :

* Appel de `CrunchConfig::parseConfigFile` : cette méthode va initialiser tous les paramètres de configuration du crunch. Elle crée les paramètres par défaut puis charge le fichier de configuration (au format JSON). Elle stocke notamment les expressions régulières à appliquer aux champs, les champs custom (qui diffèrent de la norme google) à rajouter au crunch ainsi que les règles de lecture des fichiers CSV lorsque le fichier est à ce format.
* Elle crée le repértoire de destination s'il n'existe pas
* Elle prépare la liste des champs à rechercher dans le fichier source pour chaque élément du crunch. Cette liste de champ contient notamment les expressions régulières à appliquer, les remplacements de nom de champ ... Elle est représentée par les deux attributs `Crunch::m_fieldList` et `Crunch::m_multipleFieldList` qui sont des hashmap de types `CrunchField`. La clé d'entrée de la map est le nom du champ dans la norme de google. `m_multipleFieldList` regroupe les champs qui peuvent apparaître plusieurs fois pour un item (par exemple les images)

### crunch::Crunch::startCrunch

Cette méthode réalise le crunch proprement dit :

* Création d'un objet de type `crunch::Reader` adapté au format de fichier source ( `crunch::XmlReader` ou `crunch::CSVreader`)
* Demande au Reader de télécharger le fichier source
* Pour chaque élément fourni par le reader ( `m_reader->hasNextItem()`), appel de la fonction `itemLoop` (voir ci-dessous)
* Téléchargement des photos avec la méthode `downloadPictures`
* Création des fichiers JSON du catalogue avec la méthode `outputJson`

Pour chaque élément, la méthode `itemLoop` réalise les fonctions suivantes :

* récupération des champs dans une structure `Item` intermédiaire via l'appel de la fonction `populateItem()`
* repérage ou création de la catégorie par l'appel de la fonction `findOrCreateCategory()`
* repérage ou création du modèle (on recherche s'il est déjà présent dans la hashmap m_models)
* repérage ou création du produit (on recherche s'il est déjà présent dans la hashmap du modèle courant)
* création de la référence 
* ajout de la ou les photos dans la liste des photos à télécharger
* ajout du modèle/produit/référence aux index de recherche



Change log
----





Todo
----

### TODO

- attention à la gestion des URLs de photo lorsque le catalogue est partagé. Il faut prefixer le path photo par le répertoire d'accès local
- la gestion des urls photo est fausse en lorsqu'elle sont downloadés mais non transmises
- attention , si transmit est false pour le catalog, tous les indexs de recherche doivent être false


* insérer le path de la photo dans le modèle DONE ?
* rajouter la ressource parent dans le fichier categorie ?
* le versionning du crunch est prévu dans les outputs; il faut pouvoir le paramétrer par la ligne de commande
* apres la lecture du fichier de conf et de la création des crunchfield (dans crunch::init), gérer l'init des indexs de recherche

* modifier le nom des index de recherches pour qu'ils soient urlFriendly
