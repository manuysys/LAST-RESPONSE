# LAST RESPONSE

Simulador cooperativo de rescate en desastres naturales. Primera persona, PC/Steam.
Estado actual: **Fase 1 — Prototipo** (single player, una inundación, una víctima, dos obstáculos).

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
  Player/                      Personaje, interacción y inventario
  Interaction/                 Componente interactuable, puerta bloqueada, palanca, cuerda
  Victim/                      Víctima con máquina de estados
  World/                       Director de agua, corriente, zona segura
  Game/                        GameMode y director de misión
Content/                       Assets (crear en editor; LFS)
```

## Flujo de la misión de prueba

1. **Localizar** — la víctima grita a <6 m; pasa a `Located`.
2. **Asegurar acceso** — abrir la puerta con la palanca (`PryBar`) y cruzar con cuerda (`Rope`).
3. **Liberar y asistir** — interactuar a <2 m durante 2 s; pasa a `Assisted` y sigue al jugador.
4. **Evacuar** — llevarla a la zona segura; pasa a `Evacuated` y termina la misión.

Si el agua supera la cabeza del jugador, la misión falla. El informe final muestra tiempo, víctimas, herramientas usadas y nivel máximo de agua.

## Assets a crear en el editor (no incluidos)

- `IMC_Default` + `IA_Move`, `IA_Look`, `IA_Interact`, `IA_Flashlight`, `IA_Crouch`, `IA_Sprint`.
- `BP_PlayerCharacter` (hijo de `ALRPlayerCharacter`) con el mapping context y las acciones asignadas.
- `BP_Victim`, `BP_BlockedDoor`, `BP_ToolPickup`, `BP_RopeAnchor`, `BP_RopeLine`, `BP_WaterDirector`, `BP_SafeZone`, `BP_MissionDirector`.
- Mapa `M_FloodTest` con el GameMode `LRGameMode` y los actores colocados.
