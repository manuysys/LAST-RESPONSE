# Herramienta de setup del prototipo para LAST RESPONSE.
# Ejecutar en el editor de Unreal (5.4+), con el plugin "Python Editor Script Plugin" habilitado:
#   1. Abrir LastResponse.uproject y compilar el modulo C++ (si pide rebuild, aceptar).
#   2. File > Save All (el script crea niveles nuevos y descarta el actual si no esta guardado).
#   3. Output Log > Cmd: py "<ruta al repo>/Tools/ue_prototype_setup.py"
#
# Crea dentro de /Game/Prototype:
#   - Materials/M_PrototypeColor y M_Water (materiales grises con parametro "Color").
#   - Input/IA_* + IMC_Default / IMC_Vehicle (WASD, mouse, E, F, Ctrl, Shift, R).
#   - Blueprints/BP_PlayerCharacter, BP_RescueVehicle, BP_GameMode, BP_StationGameMode.
#   - Maps/Station y Maps/M_FloodTest con geometria graybox, luces, gameplay y PlayerStart.
#
# Es idempotente: no recrea assets existentes ni pisa mapas ya creados (FORCE_REBUILD_MAPS = True si queres regenerarlos).

import unreal

PREFIX = "[LR-Prototype] "
FORCE_REBUILD_MAPS = False

ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EDITOR_ASSETS = unreal.EditorAssetLibrary
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LEVELS = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

PROTOTYPE_ROOT = "/Game/Prototype"
MATERIALS_DIR = PROTOTYPE_ROOT + "/Materials"
INPUT_DIR = PROTOTYPE_ROOT + "/Input"
BLUEPRINTS_DIR = PROTOTYPE_ROOT + "/Blueprints"
MAPS_DIR = PROTOTYPE_ROOT + "/Maps"


def log(message):
    unreal.log(PREFIX + str(message))


def warn(message):
    unreal.log_warning(PREFIX + str(message))


def fail(message):
    unreal.log_error(PREFIX + str(message))


def ensure_directory(path):
    if not EDITOR_ASSETS.does_directory_exist(path):
        EDITOR_ASSETS.make_directory(path)


def load_prototype_asset(path):
    if EDITOR_ASSETS.does_asset_exist(path):
        return EDITOR_ASSETS.load_asset(path)
    return None


def save_asset(asset):
    if asset:
        try:
            EDITOR_ASSETS.save_loaded_asset(asset)
        except Exception as exc:
            warn("No se pudo guardar {0}: {1}".format(asset, exc))


def load_basic_shape(shape_name):
    return EDITOR_ASSETS.load_asset("/Engine/BasicShapes/{0}.{0}".format(shape_name))


def resolve_class(class_name):
    cls = getattr(unreal, class_name, None)
    if cls:
        return cls
    try:
        return unreal.load_class(None, "/Script/LastResponse." + class_name)
    except Exception:
        return None


# ---------------------------------------------------------------------------
# Materiales
# ---------------------------------------------------------------------------

def create_color_material():
    ensure_directory(MATERIALS_DIR)
    full_path = MATERIALS_DIR + "/M_PrototypeColor"
    existing = load_prototype_asset(full_path)
    if existing:
        return existing

    material = ASSET_TOOLS.create_asset("M_PrototypeColor", MATERIALS_DIR, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        fail("No se pudo crear M_PrototypeColor")
        return None

    try:
        color_param = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionVectorParameter, -400, 0)
        color_param.set_editor_property("parameter_name", "Color")
        color_param.set_editor_property("default_value", unreal.LinearColor(0.75, 0.75, 0.75, 1.0))
        unreal.MaterialEditingLibrary.connect_material_property(
            color_param, "", unreal.MaterialProperty.MP_BASE_COLOR)
        unreal.MaterialEditingLibrary.recompile_material(material)
    except Exception as exc:
        warn("M_PrototypeColor: {0}".format(exc))

    save_asset(material)
    log("Material creado: M_PrototypeColor")
    return material


def create_water_material():
    ensure_directory(MATERIALS_DIR)
    full_path = MATERIALS_DIR + "/M_Water"
    existing = load_prototype_asset(full_path)
    if existing:
        return existing

    material = ASSET_TOOLS.create_asset("M_Water", MATERIALS_DIR, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        fail("No se pudo crear M_Water")
        return None

    try:
        material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        material.set_editor_property("two_sided", True)

        color_param = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionVectorParameter, -600, -200)
        color_param.set_editor_property("parameter_name", "Color")
        color_param.set_editor_property("default_value", unreal.LinearColor(0.04, 0.3, 0.5, 1.0))
        unreal.MaterialEditingLibrary.connect_material_property(
            color_param, "", unreal.MaterialProperty.MP_BASE_COLOR)

        opacity_param = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionScalarParameter, -600, 100)
        opacity_param.set_editor_property("parameter_name", "Opacity")
        opacity_param.set_editor_property("default_value", 0.55)
        unreal.MaterialEditingLibrary.connect_material_property(
            opacity_param, "", unreal.MaterialProperty.MP_OPACITY)

        unreal.MaterialEditingLibrary.recompile_material(material)
    except Exception as exc:
        warn("M_Water: {0}".format(exc))

    save_asset(material)
    log("Material creado: M_Water")
    return material


# ---------------------------------------------------------------------------
# Inputs
# ---------------------------------------------------------------------------

def create_input_action(name, value_type):
    ensure_directory(INPUT_DIR)
    full_path = INPUT_DIR + "/" + name
    existing = load_prototype_asset(full_path)
    if existing:
        return existing

    factory = unreal.InputActionFactory()
    action = ASSET_TOOLS.create_asset(name, INPUT_DIR, unreal.InputAction, factory)
    if action:
        try:
            action.set_editor_property("value_type", value_type)
        except Exception as exc:
            warn("{0}: {1}".format(name, exc))
        save_asset(action)
    return action


def make_key(key_name):
    key = unreal.Key()
    key.set_editor_property("key_name", key_name)
    return key


def make_mapping(action, key_name, negate=False, swizzle=False):
    mapping = unreal.EnhancedActionKeyMapping()
    mapping.set_editor_property("action", action)
    mapping.set_editor_property("key", make_key(key_name))

    modifiers = []
    if negate:
        modifiers.append(unreal.InputModifierNegate())
    if swizzle:
        modifiers.append(unreal.InputModifierSwizzleAxis())
    if modifiers:
        mapping.set_editor_property("modifiers", modifiers)

    return mapping


def make_axis2d_mappings(action):
    return [
        make_mapping(action, "W", swizzle=True),
        make_mapping(action, "S", negate=True, swizzle=True),
        make_mapping(action, "A", negate=True),
        make_mapping(action, "D"),
    ]


def create_mapping_context(name, mappings):
    ensure_directory(INPUT_DIR)
    full_path = INPUT_DIR + "/" + name
    existing = load_prototype_asset(full_path)
    if existing:
        return existing

    factory = unreal.InputMappingContextFactory()
    context = ASSET_TOOLS.create_asset(name, INPUT_DIR, unreal.InputMappingContext, factory)
    if context:
        try:
            context.set_editor_property("mappings", mappings)
        except Exception as exc:
            warn("{0}: {1}".format(name, exc))
        save_asset(context)
    return context


def build_inputs():
    boolean_type = unreal.InputActionValueType.BOOLEAN
    axis2d_type = unreal.InputActionValueType.AXIS2_D

    actions = {
        "IA_Move": create_input_action("IA_Move", axis2d_type),
        "IA_Look": create_input_action("IA_Look", axis2d_type),
        "IA_Interact": create_input_action("IA_Interact", boolean_type),
        "IA_Flashlight": create_input_action("IA_Flashlight", boolean_type),
        "IA_Crouch": create_input_action("IA_Crouch", boolean_type),
        "IA_Sprint": create_input_action("IA_Sprint", boolean_type),
        "IA_Drive": create_input_action("IA_Drive", axis2d_type),
        "IA_ExitVehicle": create_input_action("IA_ExitVehicle", boolean_type),
        "IA_Siren": create_input_action("IA_Siren", boolean_type),
    }

    look_mappings = [make_mapping(actions["IA_Look"], "Mouse2D")]

    default_mappings = make_axis2d_mappings(actions["IA_Move"])
    default_mappings += look_mappings
    default_mappings += [
        make_mapping(actions["IA_Interact"], "E"),
        make_mapping(actions["IA_Flashlight"], "F"),
        make_mapping(actions["IA_Crouch"], "LeftControl"),
        make_mapping(actions["IA_Sprint"], "LeftShift"),
    ]

    vehicle_mappings = make_axis2d_mappings(actions["IA_Drive"])
    vehicle_mappings += [
        make_mapping(actions["IA_ExitVehicle"], "E"),
        make_mapping(actions["IA_Siren"], "R"),
    ]

    contexts = {
        "IMC_Default": create_mapping_context("IMC_Default", default_mappings),
        "IMC_Vehicle": create_mapping_context("IMC_Vehicle", vehicle_mappings),
    }

    log("Inputs creados: IA_Move, IA_Look, IA_Interact, IA_Flashlight, IA_Crouch, IA_Sprint, IA_Drive, IA_ExitVehicle, IA_Siren, IMC_Default, IMC_Vehicle")
    return actions, contexts


# ---------------------------------------------------------------------------
# Blueprints
# ---------------------------------------------------------------------------

def create_blueprint(name, parent_class):
    ensure_directory(BLUEPRINTS_DIR)
    full_path = BLUEPRINTS_DIR + "/" + name
    existing = load_prototype_asset(full_path)
    if existing:
        return existing

    if not parent_class:
        fail("No se encontro la clase padre para " + name)
        return None

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    blueprint = ASSET_TOOLS.create_asset(name, BLUEPRINTS_DIR, unreal.Blueprint, factory)
    if blueprint:
        save_asset(blueprint)
        log("Blueprint creado: " + name)
    return blueprint


def get_blueprint_cdo(blueprint):
    if not blueprint:
        return None
    try:
        return unreal.get_default_object(blueprint.generated_class())
    except Exception:
        pass
    try:
        return blueprint.generated_class().get_default_object()
    except Exception as exc:
        warn("CDO de {0}: {1}".format(blueprint, exc))
        return None


def set_cdo_properties(blueprint, properties):
    cdo = get_blueprint_cdo(blueprint)
    if not cdo:
        return
    for key, value in properties.items():
        if value is None:
            continue
        try:
            cdo.set_editor_property(key, value)
        except Exception as exc:
            warn("CDO {0}.{1}: {2}".format(blueprint.get_name(), key, exc))
    save_asset(blueprint)


def build_blueprints(actions, contexts):
    player_bp = create_blueprint("BP_PlayerCharacter", resolve_class("LRPlayerCharacter"))
    set_cdo_properties(player_bp, {
        "default_mapping_context": contexts.get("IMC_Default"),
        "move_action": actions.get("IA_Move"),
        "look_action": actions.get("IA_Look"),
        "interact_action": actions.get("IA_Interact"),
        "flashlight_action": actions.get("IA_Flashlight"),
        "crouch_action": actions.get("IA_Crouch"),
        "sprint_action": actions.get("IA_Sprint"),
    })

    vehicle_bp = create_blueprint("BP_RescueVehicle", resolve_class("LRRescueVehicle"))
    set_cdo_properties(vehicle_bp, {
        "vehicle_mapping_context": contexts.get("IMC_Vehicle"),
        "drive_action": actions.get("IA_Drive"),
        "exit_action": actions.get("IA_ExitVehicle"),
        "siren_action": actions.get("IA_Siren"),
    })

    game_mode_bp = create_blueprint("BP_GameMode", resolve_class("LRGameMode"))
    set_cdo_properties(game_mode_bp, {
        "default_pawn_class": player_bp.generated_class() if player_bp else None,
    })

    station_mode_bp = create_blueprint("BP_StationGameMode", resolve_class("LRStationGameMode"))
    set_cdo_properties(station_mode_bp, {
        "default_pawn_class": player_bp.generated_class() if player_bp else None,
    })

    return player_bp, vehicle_bp, game_mode_bp, station_mode_bp


# ---------------------------------------------------------------------------
# Helpers de niveles
# ---------------------------------------------------------------------------

def apply_world_gamemode(game_mode_class):
    if not game_mode_class:
        return
    try:
        for actor in ACTORS.get_all_level_actors():
            if isinstance(actor, unreal.WorldSettings):
                actor.set_editor_property("default_game_mode", game_mode_class)
                return
    except Exception as exc:
        warn("WorldSettings: {0}".format(exc))


def setup_lighting(sun_intensity=5.0):
    try:
        sun = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 3000), unreal.Rotator(-48, 140, 0))
        if sun:
            sun.set_actor_label("Sun")
            sun.light_component.set_editor_property("intensity", sun_intensity)
        ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 800)).set_actor_label("SkyLight")
        ACTORS.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0)).set_actor_label("SkyAtmosphere")
        ACTORS.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 500)).set_actor_label("Fog")
    except Exception as exc:
        warn("Luces: {0}".format(exc))


def spawn_solid(shape_name, location, scale, rotation=(0, 0, 0), label=None):
    mesh = load_basic_shape(shape_name)
    if not mesh:
        return None

    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location), unreal.Rotator(*rotation))
    if not actor:
        return None

    try:
        actor.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
        actor.static_mesh_component.set_static_mesh(mesh)
        actor.set_actor_scale3d(unreal.Vector(*scale))
        actor.set_actor_label(label or shape_name)
    except Exception as exc:
        warn("Solid {0}: {1}".format(shape_name, exc))
    return actor


def spawn_actor(actor_class, location, rotation=(0, 0, 0), label=None):
    if not actor_class:
        return None
    actor = ACTORS.spawn_actor_from_class(actor_class, unreal.Vector(*location), unreal.Rotator(*rotation))
    if actor and label:
        actor.set_actor_label(label)
    return actor


def set_property_safe(actor, key, value, context):
    if not actor:
        return False
    try:
        actor.set_editor_property(key, value)
        return True
    except Exception as exc:
        warn("{0}.{1}: {2}".format(context, key, exc))
        return False


def begin_level(level_path, game_mode_class, sun_intensity):
    ensure_directory(MAPS_DIR)

    if EDITOR_ASSETS.does_asset_exist(level_path):
        if not FORCE_REBUILD_MAPS:
            log("El mapa ya existe, no se regenera: " + level_path)
            return False
        try:
            EDITOR_ASSETS.delete_asset(level_path)
        except Exception as exc:
            warn("No se pudo borrar {0}: {1}".format(level_path, exc))
            return False

    try:
        if not LEVELS.new_level(level_path):
            fail("new_level devolvio False para " + level_path)
            return False
    except Exception as exc:
        fail("No se pudo crear {0}: {1}".format(level_path, exc))
        return False

    apply_world_gamemode(game_mode_class)
    setup_lighting(sun_intensity)
    return True


def finish_level(level_path):
    try:
        LEVELS.save_current_level()
        log("Mapa guardado: " + level_path)
    except Exception as exc:
        fail("No se pudo guardar {0}: {1}".format(level_path, exc))


# ---------------------------------------------------------------------------
# Mapa Station
# ---------------------------------------------------------------------------

def make_contract(contract_id, title, description, water_level, rise_speed, limit_seconds):
    contract = unreal.Contract()
    set_property_safe(contract, "contract_id", contract_id, "FContract")
    set_property_safe(contract, "title", title, "FContract")
    set_property_safe(contract, "description", description, "FContract")
    set_property_safe(contract, "map_name", "M_FloodTest", "FContract")
    set_property_safe(contract, "water_level", water_level, "FContract")
    set_property_safe(contract, "rise_speed", rise_speed, "FContract")
    set_property_safe(contract, "expected_victims", 2, "FContract")
    set_property_safe(contract, "time_limit_seconds", limit_seconds, "FContract")
    set_property_safe(contract, "reward_budget", 500, "FContract")
    set_property_safe(contract, "reward_reputation", 20, "FContract")
    return contract


def build_station_map(game_mode_class, vehicle_class):
    level_path = MAPS_DIR + "/Station"
    if not begin_level(level_path, game_mode_class, 5.0):
        return

    spawn_solid("Cube", (0, 0, -10), (30, 20, 0.2), label="Floor")
    spawn_solid("Cube", (0, 1000, 300), (30, 0.2, 6), label="WallNorth")
    spawn_solid("Cube", (0, -1000, 300), (30, 0.2, 6), label="WallSouth")
    spawn_solid("Cube", (1500, 0, 300), (0.2, 20, 6), label="WallEast")
    spawn_solid("Cube", (-1500, 0, 300), (0.2, 20, 6), label="WallWest")

    spawn_actor(unreal.PlayerStart, (0, -1200, 100), label="PlayerStart")

    station_gm = resolve_class("LRStationGameMode")

    board = spawn_actor(resolve_class("LRAlertBoard"), (-500, -600, 150), label="AlertBoard")
    if board:
        contracts = [
            make_contract("C1", "Rescate en el barrio", "Dos victimas atrapadas por la crecida.", 40.0, 2.0, 900.0),
            make_contract("C2", "Crecida rapida", "Corriente fuerte: usa la cuerda.", 60.0, 4.0, 600.0),
        ]
        set_property_safe(board, "contracts", contracts, "AlertBoard")

    next_button = spawn_actor(resolve_class("LRStationButton"), (-300, -450, 15), label="Button_NextContract")
    set_property_safe(next_button, "target_actor", board, "Button_Next")
    button_action_enum = getattr(unreal, "StationButtonAction", None)
    if button_action_enum and next_button:
        set_property_safe(next_button, "action", button_action_enum.BOARD_NEXT, "Button_Next")

    accept_button = spawn_actor(resolve_class("LRStationButton"), (-150, -450, 15), label="Button_AcceptContract")
    set_property_safe(accept_button, "target_actor", board, "Button_Accept")
    if button_action_enum and accept_button:
        set_property_safe(accept_button, "action", button_action_enum.BOARD_ACCEPT, "Button_Accept")

    spawn_actor(resolve_class("LRLoadoutLocker"), (400, -700, 100), label="LoadoutLocker")
    workshop = spawn_actor(resolve_class("LRWorkshop"), (1100, -700, 80), label="Workshop")

    repair_button = spawn_actor(resolve_class("LRStationButton"), (950, -450, 15), label="Button_Repair")
    set_property_safe(repair_button, "target_actor", workshop, "Button_Repair")
    if button_action_enum and repair_button:
        set_property_safe(repair_button, "action", button_action_enum.WORKSHOP_REPAIR, "Button_Repair")

    buy_button = spawn_actor(resolve_class("LRStationButton"), (1250, -450, 15), label="Button_Buy")
    set_property_safe(buy_button, "target_actor", workshop, "Button_Buy")
    if button_action_enum and buy_button:
        set_property_safe(buy_button, "action", button_action_enum.WORKSHOP_BUY, "Button_Buy")

    spawn_actor(vehicle_class, (1100, 300, 10), (0, 180, 0), label="RescueVehicle")

    finish_level(level_path)


# ---------------------------------------------------------------------------
# Mapa M_FloodTest
# ---------------------------------------------------------------------------

def build_flood_map(game_mode_class):
    level_path = MAPS_DIR + "/M_FloodTest"
    if not begin_level(level_path, game_mode_class, 2.0):
        return

    spawn_solid("Cube", (0, 0, -20), (100, 100, 0.4), label="Ground")

    spawn_solid("Cube", (0, 1200, 300), (16, 0.4, 6), label="Building_Back")
    spawn_solid("Cube", (-800, 400, 300), (0.4, 16, 6), label="Building_Left")
    spawn_solid("Cube", (800, 400, 300), (0.4, 16, 6), label="Building_Right")
    spawn_solid("Cube", (-435, -400, 300), (7.3, 0.4, 6), label="Building_FrontL")
    spawn_solid("Cube", (435, -400, 300), (7.3, 0.4, 6), label="Building_FrontR")

    spawn_solid("Cube", (2600, -400, 150), (0.6, 16, 3), label="Canal_WallA")
    spawn_solid("Cube", (2600, 1200, 150), (0.6, 16, 3), label="Canal_WallB")

    spawn_actor(unreal.PlayerStart, (0, -4200, 120), label="PlayerStart")

    door = spawn_actor(resolve_class("LRBlockedDoor"), (0, -400, 105), label="BlockedDoor")

    victim_critical = spawn_actor(resolve_class("LRVictimCharacter"), (-300, 300, 100), label="Victim_Critical")
    victim_condition_enum = getattr(unreal, "VictimCondition", None)
    if victim_condition_enum and victim_critical:
        set_property_safe(victim_critical, "condition", victim_condition_enum.CRITICAL, "Victim_Critical")

    victim_trapped = spawn_actor(resolve_class("LRVictimCharacter"), (300, 700, 100), label="Victim_Trapped")
    if victim_condition_enum and victim_trapped:
        set_property_safe(victim_trapped, "condition", victim_condition_enum.TRAPPED, "Victim_Trapped")

    spawn_actor(resolve_class("LRHeavyDebris"), (-600, -100, 45), label="HeavyDebris")

    prybar = spawn_actor(resolve_class("LRToolPickup"), (0, -2600, 30), label="Pickup_PryBar")
    set_property_safe(prybar, "tool_id", "PryBar", "Pickup_PryBar")

    medkit = spawn_actor(resolve_class("LRToolPickup"), (-500, -4500, 30), label="Pickup_Medkit")
    set_property_safe(medkit, "tool_id", "Medkit", "Pickup_Medkit")

    rope = spawn_actor(resolve_class("LRToolPickup"), (1500, -900, 30), label="Pickup_Rope")
    set_property_safe(rope, "tool_id", "Rope", "Pickup_Rope")

    anchor_a = spawn_actor(resolve_class("LRRopeAnchor"), (1900, -800, 70), label="RopeAnchor_A")
    anchor_b = spawn_actor(resolve_class("LRRopeAnchor"), (1900, 800, 70), label="RopeAnchor_B")
    set_property_safe(anchor_a, "linked_anchor", anchor_b, "RopeAnchor_A")
    set_property_safe(anchor_b, "linked_anchor", anchor_a, "RopeAnchor_B")

    current_zone = spawn_actor(resolve_class("LRCurrentZone"), (2600, 400, 200), label="CurrentZone")

    spawn_actor(resolve_class("LRSafeZone"), (0, -3800, 200), label="SafeZone")

    spawn_actor(resolve_class("LRStormDirector"), (0, 0, 0), label="StormDirector")
    water = spawn_actor(resolve_class("LRWaterDirector"), (0, 0, 0), label="WaterDirector")

    director = spawn_actor(resolve_class("LRMissionDirector"), (0, 0, 0), label="MissionDirector")
    if director:
        victims = [victim_critical, victim_trapped]
        set_property_safe(director, "victims", victims, "MissionDirector")
        set_property_safe(director, "water_director", water, "MissionDirector")

    finish_level(level_path)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    log("Inicio del setup del prototipo...")

    create_color_material()
    create_water_material()

    actions, contexts = build_inputs()
    player_bp, vehicle_bp, game_mode_bp, station_mode_bp = build_blueprints(actions, contexts)

    vehicle_spawn_class = vehicle_bp.generated_class() if vehicle_bp else resolve_class("LRRescueVehicle")

    build_station_map(
        station_mode_bp.generated_class() if station_mode_bp else resolve_class("LRStationGameMode"),
        vehicle_spawn_class)

    build_flood_map(game_mode_bp.generated_class() if game_mode_bp else resolve_class("LRGameMode"))

    log("Listo. Podes abrir /Game/Prototype/Maps/Station y /Game/Prototype/Maps/M_FloodTest y darle Play.")


main()
