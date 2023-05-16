# RoadTilesRecognition

## Tileset
<div style="text-align:left">
  <img src="data/road_tiles/tile1.jpg" width="160">
  <img src="data/road_tiles/tile2.jpg" width="160">
  <img src="data/road_tiles/tile3.jpg" width="160">
  <img src="data/road_tiles/tile4.jpg" width="160">
  <img src="data/road_tiles/tile5.jpg" width="160">
</div>

# Definition
Projet de reconnaissance de t

## Software Robot
Reconnaissance route (tensorflow lite):
- Tiles Position Multiple (R-CNN)
- Data augmentation

Moteur interne:
le robot doit être capable de générer une tileMap à partir de son module de détection de tiles
- module de gestion tileMap basique
- moteur physique basique

## Hardware Robot
- Le robot est piloté par un ESP32-CAM
- Le chassis est un simple PCB 60x40mm
- Le robot possède deux roues et doit donc se maintenir en équilibre (GY-6500)
- 2 moteurs stepper + drivers (DRV8833/L298N)
- L'alimentation se fait par un super-cap 60F + mini convertisseur boost (DD0405MA)

  <img src="data/modules.jpg" width="400">
  
  
