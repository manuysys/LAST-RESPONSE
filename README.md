# LAST RESPONSE

Simulador cooperativo de rescate en desastres naturales. Primera persona, PC/Steam.
Estado actual: **Fases 1–3 implementadas en C++** (prototipo single player, cooperativo 2–4, vertical slice con estación y vehículo). Fase 4 aporta herramientas de testeo.

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

## Assets a crear en el editor (no incluidos)

- `IMC_Default` + `IA_Move`, `IA_Look`, `IA_Interact`, `IA_Flashlight`, `IA_Crouch`, `IA_Sprint`; `IMC_Vehicle` + `IA_Drive` (Vector2D), `IA_VehicleExit`, `IA_Siren`.
- `BP_PlayerCharacter` (hijo de `ALRPlayerCharacter`) con el mapping context y las acciones asignadas.
- `BP_Victim`, `BP_BlockedDoor`, `BP_HeavyDebris`, `BP_ToolPickup`, `BP_RopeAnchor`, `BP_RopeLine`, `BP_WaterDirector`, `BP_CurrentZone`, `BP_SafeZone`, `BP_StormDirector`, `BP_MissionDirector`.
- `BP_RescueVehicle` (hijo de `ALRRescueVehicle`) con malla, mapping de vehículo y sonido de sirena.
- `BP_AlertBoard`, `BP_LoadoutLocker`, `BP_Workshop` con contratos y ofertas configurados.
- Mapas `Station` (GameMode `LRStationGameMode`) y `M_FloodTest` (GameMode `LRGameMode`) con los actores colocados.
