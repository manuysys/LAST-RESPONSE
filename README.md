# LAST RESPONSE

Simulador cooperativo de rescate en desastres naturales. Primera persona, PC/Steam.
Estado actual: **Fases 1–4 implementadas en C++** (prototipo single player, cooperativo 2–4, vertical slice con estación y vehículo, herramientas de testeo) + **graybox visual** (todos los actores tienen mallas y colores por código) + **pipeline de modelos reales**: `Tools/generate_prototype_meshes.py` genera 15 modelos low-poly (.obj/.mtl) y el juego los usa automáticamente si existen en `/Game/Prototype/Meshes` (si no, cae al graybox). `Tools/ue_prototype_setup.py` genera materiales, inputs, Blueprints, importa los modelos y arma los mapas de prueba en un paso.

## Requisitos

- Unreal Engine 5.4 o superior (con C++ toolchain de Visual Studio 2022 en Windows).
- Git LFS (`git lfs install`) antes de agregar assets binarios.

## Abrir el proyecto

1. Click derecho en `LastResponse.uproject` → *Generate Visual Studio project files*.
2. Abrir `LastResponse.sln`, compilar en `Development Editor`.
3. Abrir el proyecto en Unreal. La asociación de motor está vacía a propósito: el launcher pedirá elegir la versión instalada.

## Estructura

```
Config/                        Configuración del proyecto (Enhanced Input por defecto)
Source/LastResponse/
  LRTypes.h                    Enums y structs compartidos (estados de víctima, informe)
  Player/                      Personaje, interacción (RPCs servidor) e inventario replicado
  Interaction/                 Interactuable base, coop, puerta, escombros, palanca, cuerda
  Victim/                      Víctima con máquina de estados y condición (Stable→Trapped)
  World/                       Agua replicada, corriente, zona segura, tormenta
  Vehicle/                     Camioneta arcade (conductor + 3 pasajeros, daño, sirena)
  Station/                     Tablero de contratos, armero con peso, taller
  Meta/                        Contratos, progresión (presupuesto/reputación), guardado
  Game/                        GameInstance (host/join), GameMode, director de misión
  Debug/                       Cheat manager y grabador de sesiones de playtest
Content/                       Assets (crear en editor; LFS)
```

## Flujo de la misión de prueba

1. **Localizar** — la víctima grita a <6 m; pasa a `Located`.
2. **Asegurar acceso** — abrir la puerta con la palanca (`PryBar`) y cruzar con cuerda (`Rope`, asegurando las dos anclas).
3. **Liberar y asistir** — interactuar a <2 m durante 2 s; pasa a `Assisted` y sigue al jugador.
4. **Evacuar** — llevarla a la zona segura; pasa a `Evacuated` y termina la misión.

Si el agua supera la cabeza del jugador, en single player la misión falla; en cooperativo el jugador queda caído y un compañero debe reanimarlo (al terminar la reanimación el caído es arrastrado junto al reanimador, fuera del agua; si caen todos, la misión falla). El informe final muestra tiempo, víctimas, objetivos, herramientas usadas, daño al vehículo, nivel máximo de agua y recompensas.

## Cooperativo (Fase 2)

- Interacciones validadas en servidor con predicción local en cliente.
- Escombros pesados (`BP_HeavyDebris`) que exigen 2 jugadores simultáneos.
- Agua, misión, inventario y estados replicados. Viaje fluido entre estación y misión.
- Reanimación cooperativa: el caído es expulsado del vehículo si aplica y reaparece junto al reanimador.
- La línea de cuerda se crea recién cuando **ambas** anclas están aseguradas (linkear las anclas mutuamente en el nivel).
- El vehículo solo permite entrar si hay asiento libre (conteo replicado).

## Vertical slice (Fase 3)

- Estación: tablero de contratos (`BP_AlertBoard`), armero con límite de peso (`BP_LoadoutLocker`), taller (`BP_Workshop`) con reparación y compras. Progreso persistente (presupuesto, reputación, condición del vehículo).
- Camioneta arcade: conductor + 3 pasajeros, sirena, daño por choques que afecta la evaluación.
- Tormenta: lluvia, viento (afecta corrientes) y rayos con evento para VFX/sonido.
- Víctimas con condición: `Critical` exige botiquín; cada condición cambia tiempo de asistencia y velocidad de escolta.

## Testeo (Fase 4)

- Consola: `LR_SetWater`, `LR_ForceRise`, `LR_GiveTool`, `LR_WinMission`, `LR_TeleportToVictim`, `LR_CompleteObjective`, `LR_SetStorm`, `LR_AddBudget` (GameMode ya usa este cheat manager). Los cheats viajan al servidor, así que funcionan también desde clientes.
- Cada misión genera un CSV en `Saved/Playtest/` con eventos (víctimas, herramientas, objetivos, resultado) y tiempos relativos al inicio de la sesión.

## Graybox (cómo se ve el prototipo)

El C++ asigna mallas primitivas del engine (`/Engine/BasicShapes`) y colores por instancia (versión dinámica de `M_PrototypeColor`) para que todo se vea sin assets externos: jugador (naranja, oculto en primera persona) y víctimas (rojas) con cuerpo y cabeza, camioneta (chasis rojo, cabina blanca, ruedas y baliza), puerta, escombros, anclas y cuerda fina, pickups, tablero, armero, taller, botones, agua translúcida, corriente y zona segura (pad verde). Si los materiales de `/Game/Prototype/Materials` todavía no existen se usa `BasicShapeMaterial` del engine (gris), así que el proyecto corre igual.

## Modelos reales (pipeline de 3 capas)

1. **Generador**: `python Tools/generate_prototype_meshes.py` escribe 15 modelos low-poly propios (.obj + .mtl, en centímetros) en `Art/PrototypeMeshes/`: `SM_Player`, `SM_Victim`, `SM_Truck` (con ruedas y baliza), `SM_Door` (pivot en la bisagra), `SM_Debris`, `SM_Anchor`, `SM_Tool_PryBar`, `SM_Tool_Medkit`, `SM_Tool_Rope`, `SM_AlertBoard`, `SM_Locker`, `SM_Workshop`, `SM_Button`, `SM_ZonePad` y `SM_CurrentPad`. Solo necesita Python 3, sin dependencias.
2. **Importación**: `Tools/ue_prototype_setup.py` los importa a `/Game/Prototype/Meshes` automáticamente (soporta `.obj`, `.fbx`, `.glb`, `.gltf`).
3. **Uso en juego**: `FLRProto::UpgradeToRealModel` (C++) asigna el modelo al actor en `BeginPlay`; si el modelo tiene materiales propios, se descarta el color graybox; si el asset no existe, el actor queda como estaba. Agua y cuerda no tienen modelo (siguen siendo primitivas).

**¿Querés mejores modelos?** Reemplazá cualquier archivo de `Art/PrototypeMeshes/` respetando el nombre (por ejemplo `SM_Truck.glb`) con uno generado con IA —Meshy, Tripo, Hunyuan3D/TRELLIS (Hugging Face Spaces)— o un pack gratis (Kenney, Fab) y volvé a correr `Tools/ue_prototype_setup.py`: no hace falta tocar C++. Convención de pivots: personajes y props apoyados en el suelo llevan el pivot en el centro de la base; la puerta lo lleva en la bisagra (borde X = 0, altura centrada).

## Setup automático del editor

`Tools/ue_prototype_setup.py` crea todo lo que antes había que armar a mano (requiere el plugin *Python Editor Script Plugin*):

1. Materiales `M_PrototypeColor` (parámetro `Color`) y `M_Water` (parámetros `Color` y `Opacity`, translúcido).
2. Inputs: `IMC_Default` (WASD, mouse, E, F, Ctrl, Shift) e `IMC_Vehicle` (WASD, E salir, R sirena) con sus `IA_*`.
3. Blueprints: `BP_PlayerCharacter` y `BP_RescueVehicle` con los inputs asignados; `BP_GameMode` y `BP_StationGameMode` usando ese pawn.
4. Modelos de `Art/PrototypeMeshes/` importados a `/Game/Prototype/Meshes` (si no hay archivos, el juego usa el graybox).
5. Mapas `Station` y `M_FloodTest` con geometría graybox, luces, víctimas (`Critical`/`Trapped`), puerta con palanca, escombros coop, cuerdas (anclas linkeadas), pickups, corriente, zona segura, directores y `PlayerStart`, con el GameMode ya configurado.

Uso: compilar C++ (paso 2 de *Abrir el proyecto*), guardar todo y ejecutar en el Output Log:

```
py "<ruta al repo>/Tools/ue_prototype_setup.py"
```

Es idempotente (no pisa assets ni mapas existentes; poner `FORCE_REBUILD_MAPS = True` para regenerar los mapas). `Config/DefaultGame.ini` incluye `/Game/Prototype` en `DirectoriesToAlwaysCook` para que se hornee en builds empaquetadas.
