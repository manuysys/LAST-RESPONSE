#!/usr/bin/env python3
"""Genera modelos low-poly (.obj + .mtl) para el prototipo de LAST RESPONSE.

Uso (no necesita Unreal, cualquier Python 3):
    python Tools/generate_prototype_meshes.py

Salida: Art/PrototypeMeshes/*.obj + *.mtl. Unidades en centimetros (1 unidad = 1 cm).
Convencion de pivots:
    Personajes y props apoyados en el suelo: pivot en el centro de la base (Z = 0).
    Puerta: pivot en la bisagra (X = 0, centro vertical: Z de -105 a 105).
Los nombres coinciden exactamente con los que busca el C++ (FLRProto::UpgradeToRealModel):
si los assets existen en /Game/Prototype/Meshes, los actores usan el modelo real; si no,
siguen con el graybox de primitivas.

Para importarlos: correr Tools/ue_prototype_setup.py dentro del editor (los importa solos).
Tambien podes reemplazar cualquiera de estos .obj por un modelo generado con IA
(Meshy, Tripo, Hunyuan3D en Hugging Face, etc.) o un pack de Kenney, siempre que
respete el nombre (SM_Player.obj, SM_Truck.obj, ...). Formatos soportados por el
script de importacion: .obj, .fbx, .glb, .gltf.
"""

import math
import os

OUT_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Art", "PrototypeMeshes")

# Paleta (r, g, b en 0..1)
SUIT = (0.86, 0.36, 0.08)
SUIT_DARK = (0.55, 0.24, 0.07)
SKIN = (0.90, 0.72, 0.55)
GLOVES = (0.16, 0.16, 0.18)
BOOTS = (0.12, 0.12, 0.14)
CAP = (0.25, 0.28, 0.34)
VICTIM_SHIRT = (0.70, 0.10, 0.10)
VICTIM_PANTS = (0.20, 0.20, 0.24)
VICTIM_HAIR = (0.22, 0.16, 0.12)
TRUCK_RED = (0.72, 0.07, 0.07)
TRUCK_WHITE = (0.90, 0.90, 0.88)
GLASS = (0.16, 0.22, 0.30)
TIRE = (0.06, 0.06, 0.06)
RIM = (0.55, 0.57, 0.60)
BEACON = (0.95, 0.50, 0.05)
DOOR = (0.42, 0.26, 0.12)
DOOR_FRAME = (0.28, 0.16, 0.07)
DOOR_PANEL = (0.50, 0.32, 0.16)
HANDLE = (0.80, 0.70, 0.25)
CONCRETE = (0.55, 0.56, 0.58)
CONCRETE_DARK = (0.42, 0.43, 0.46)
REBAR = (0.40, 0.30, 0.24)
YELLOW = (0.90, 0.75, 0.10)
METAL = (0.50, 0.53, 0.56)
METAL_DARK = (0.30, 0.32, 0.35)
MEDKIT_WHITE = (0.92, 0.92, 0.90)
MEDKIT_RED = (0.80, 0.08, 0.08)
BOARD_BODY = (0.12, 0.14, 0.22)
BOARD_SCREEN = (0.25, 0.75, 0.95)
BOARD_TRIM = (0.90, 0.65, 0.10)
LOCKER_GREEN = (0.16, 0.40, 0.22)
LOCKER_DARK = (0.09, 0.24, 0.13)
WOOD_TAN = (0.48, 0.42, 0.26)
WOOD_LIGHT = (0.55, 0.48, 0.30)
TOOLBOX_RED = (0.65, 0.12, 0.10)
BUTTON_RED = (0.90, 0.12, 0.10)
BUTTON_BASE = (0.22, 0.22, 0.25)
PAD_GREEN = (0.10, 0.70, 0.30)
PAD_WHITE = (0.88, 0.92, 0.88)
PAD_BLUE = (0.12, 0.50, 0.85)
PAD_BLUE_DARK = (0.08, 0.30, 0.55)

TWO_PI = math.pi * 2.0


class ObjBuilder:
    def __init__(self, name):
        self.name = name
        self.vertices = []
        self.faces = []
        self.materials = {}

    def use(self, material_name, rgb):
        if material_name not in self.materials:
            self.materials[material_name] = rgb
        return material_name

    def _vertex(self, x, y, z):
        self.vertices.append((x, y, z))
        return len(self.vertices)

    def quad(self, material, points):
        self.faces.append((material, [self._vertex(*p) for p in points]))

    def box(self, material, p0, p1, yaw=0.0, pitch=0.0):
        x0, y0, z0 = p0
        x1, y1, z1 = p1
        corners = [
            (x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
            (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1),
        ]
        if yaw or pitch:
            cx, cy, cz = (x0 + x1) * 0.5, (y0 + y1) * 0.5, (z0 + z1) * 0.5
            corners = [self._rotate(p, cx, cy, cz, yaw, pitch) for p in corners]
        for face in ((0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (3, 7, 6, 2), (0, 4, 7, 3), (1, 2, 6, 5)):
            self.quad(material, [corners[i] for i in face])

    @staticmethod
    def _rotate(point, cx, cy, cz, yaw, pitch):
        x, y, z = point[0] - cx, point[1] - cy, point[2] - cz
        if yaw:
            cos_y, sin_y = math.cos(yaw), math.sin(yaw)
            x, y = x * cos_y - y * sin_y, x * sin_y + y * cos_y
        if pitch:
            cos_p, sin_p = math.cos(pitch), math.sin(pitch)
            x, z = x * cos_p + z * sin_p, -x * sin_p + z * cos_p
        return (x + cx, y + cy, z + cz)

    def cylinder_z(self, material, cx, cy, z0, z1, radius, segments=12):
        top_center = self._vertex(cx, cy, z1)
        bottom_center = self._vertex(cx, cy, z0)
        ring_bottom = []
        ring_top = []
        for i in range(segments):
            angle = TWO_PI * i / segments
            rx = cx + radius * math.cos(angle)
            ry = cy + radius * math.sin(angle)
            ring_bottom.append(self._vertex(rx, ry, z0))
            ring_top.append(self._vertex(rx, ry, z1))
        for i in range(segments):
            nxt = (i + 1) % segments
            self.faces.append((material, [ring_bottom[i], ring_bottom[nxt], ring_top[nxt], ring_top[i]]))
            self.faces.append((material, [ring_top[i], ring_top[nxt], top_center]))
            self.faces.append((material, [ring_bottom[i], bottom_center, ring_bottom[nxt]]))

    def cylinder_y(self, material, cx, cy0, cy1, cz, radius, segments=12):
        top_center = self._vertex(cx, cy1, cz)
        bottom_center = self._vertex(cx, cy0, cz)
        ring_bottom = []
        ring_top = []
        for i in range(segments):
            angle = TWO_PI * i / segments
            rx = cx + radius * math.cos(angle)
            rz = cz + radius * math.sin(angle)
            ring_bottom.append(self._vertex(rx, cy0, rz))
            ring_top.append(self._vertex(rx, cy1, rz))
        for i in range(segments):
            nxt = (i + 1) % segments
            self.faces.append((material, [ring_bottom[i], ring_top[i], ring_top[nxt], ring_bottom[nxt]]))
            self.faces.append((material, [ring_top[i], ring_top[nxt], top_center]))
            self.faces.append((material, [ring_bottom[i], bottom_center, ring_bottom[nxt]]))

    def write(self, out_dir):
        max_index = len(self.vertices)
        for _, face in self.faces:
            for index in face:
                if not 1 <= index <= max_index:
                    raise ValueError("Indice invalido en {0}".format(self.name))

        os.makedirs(out_dir, exist_ok=True)

        with open(os.path.join(out_dir, self.name + ".mtl"), "w") as handle:
            for material_name, rgb in sorted(self.materials.items()):
                handle.write("newmtl {0}\n".format(material_name))
                handle.write("Kd {0:.4f} {1:.4f} {2:.4f}\n".format(*rgb))
                handle.write("Ka 0.0000 0.0000 0.0000\nKs 0.0000 0.0000 0.0000\nd 1.0\nillum 1\n\n")

        with open(os.path.join(out_dir, self.name + ".obj"), "w") as handle:
            handle.write("# LAST RESPONSE - modelo generado por Tools/generate_prototype_meshes.py\n")
            handle.write("mtllib {0}.mtl\n".format(self.name))
            for x, y, z in self.vertices:
                handle.write("v {0:.3f} {1:.3f} {2:.3f}\n".format(x, y, z))
            current = None
            for material_name, face in self.faces:
                if material_name != current:
                    handle.write("usemtl {0}\n".format(material_name))
                    current = material_name
                handle.write("f " + " ".join(str(i) for i in face) + "\n")


def build_humanoid(name, torso_mat, torso_color, leg_mat, leg_color, hand_mat, hand_color, hat_mat, hat_color, vest=None, vest_color=None):
    obj = ObjBuilder(name)
    obj.use(torso_mat, torso_color)
    obj.use(leg_mat, leg_color)
    obj.use(hand_mat, hand_color)
    obj.use(hat_mat, hat_color)
    obj.use("Skin", SKIN)
    if vest:
        obj.use(vest, vest_color)

    obj.box(hand_mat, (-16, -10, 0), (-2, 10, 14))
    obj.box(hand_mat, (2, -10, 0), (16, 10, 14))
    obj.box(leg_mat, (-16, -9, 14), (-2, 9, 86))
    obj.box(leg_mat, (2, -9, 14), (16, 9, 86))
    obj.box(torso_mat, (-21, -13, 86), (21, 13, 142))
    if vest:
        obj.box(vest, (-22.5, -14.5, 98), (22.5, 14.5, 138))
    obj.box(torso_mat, (-33, -9, 86), (-22, 9, 140))
    obj.box(torso_mat, (22, -9, 86), (33, 9, 140))
    obj.box("Skin", (-33, -9, 74), (-22, 9, 86))
    obj.box("Skin", (22, -9, 74), (33, 9, 86))
    obj.box("Skin", (-6, -6, 142), (6, 6, 148))
    obj.box("Skin", (-13, -11, 148), (13, 11, 172))
    obj.box(hat_mat, (-14, -12, 164), (14, 12, 173))
    obj.box(hat_mat, (-14, -19, 164), (14, -11, 167))
    return obj


def build_player():
    return build_humanoid("SM_Player", "Suit", SUIT, "Suit", SUIT, "Gloves", GLOVES, "Cap", CAP, vest="Vest", vest_color=SUIT_DARK)


def build_victim():
    return build_humanoid("SM_Victim", "Shirt", VICTIM_SHIRT, "Pants", VICTIM_PANTS, "Skin", SKIN, "Hair", VICTIM_HAIR)


def build_truck():
    obj = ObjBuilder("SM_Truck")
    obj.use("TruckRed", TRUCK_RED)
    obj.use("TruckWhite", TRUCK_WHITE)
    obj.use("Glass", GLASS)
    obj.use("Tire", TIRE)
    obj.use("Rim", RIM)
    obj.use("Beacon", BEACON)
    obj.use("Bumper", METAL_DARK)

    obj.box("TruckRed", (-230, -100, 40), (230, 100, 150))
    obj.box("TruckRed", (60, -96, 100), (230, 96, 152))
    obj.box("TruckWhite", (-210, -96, 150), (40, 96, 230))
    obj.box("Glass", (34, -90, 155), (48, 90, 225))
    obj.box("Glass", (-200, 96, 160), (20, 102, 220))
    obj.box("Glass", (-200, -102, 160), (20, -96, 220))
    obj.box("Glass", (-216, -90, 160), (-210, 90, 220))
    obj.box("Beacon", (-150, -20, 230), (-90, 20, 256))
    obj.box("Bumper", (230, -90, 45), (236, 90, 80))
    obj.box("Bumper", (-236, -90, 45), (-230, 90, 80))

    for wheel_x in (150, -150):
        obj.cylinder_y("Tire", wheel_x, -113, -89, 38, 38)
        obj.cylinder_y("Tire", wheel_x, 89, 113, 38, 38)
        obj.cylinder_y("Rim", wheel_x, -116, -113, 38, 20)
        obj.cylinder_y("Rim", wheel_x, 113, 116, 38, 20)
    return obj


def build_door():
    obj = ObjBuilder("SM_Door")
    obj.use("Door", DOOR)
    obj.use("DoorFrame", DOOR_FRAME)
    obj.use("DoorPanel", DOOR_PANEL)
    obj.use("Handle", HANDLE)

    obj.box("Door", (0, -6, -105), (110, 6, 105))
    obj.box("DoorFrame", (0, -8, 95), (110, 8, 105))
    obj.box("DoorFrame", (0, -8, -105), (110, 8, -95))
    obj.box("DoorFrame", (0, -8, -105), (8, 8, 105))
    obj.box("DoorFrame", (102, -8, -105), (110, 8, 105))
    obj.box("DoorPanel", (14, 6, -88), (96, 8, -12))
    obj.box("DoorPanel", (14, 6, 12), (96, 8, 88))
    obj.box("DoorPanel", (14, -8, -88), (96, -6, -12))
    obj.box("DoorPanel", (14, -8, 12), (96, -6, 88))
    obj.box("Handle", (88, 8, -8), (104, 16, 8))
    return obj


def build_debris():
    obj = ObjBuilder("SM_Debris")
    obj.use("Concrete", CONCRETE)
    obj.use("ConcreteDark", CONCRETE_DARK)
    obj.use("Rebar", REBAR)

    obj.box("ConcreteDark", (-120, -90, 0), (120, 90, 30))
    obj.box("Concrete", (-95, -55, 30), (-5, 35, 88), yaw=math.radians(12))
    obj.box("Concrete", (-40, -80, 30), (60, 10, 70), yaw=math.radians(-18))
    obj.box("ConcreteDark", (20, -30, 30), (110, 60, 60), yaw=math.radians(25))
    obj.box("Rebar", (-70, -15, 85), (70, -5, 92), yaw=math.radians(30))
    obj.box("Rebar", (-60, 5, 78), (80, 15, 85), yaw=math.radians(-35))
    return obj


def build_anchor():
    obj = ObjBuilder("SM_Anchor")
    obj.use("Yellow", YELLOW)
    obj.use("MetalDark", METAL_DARK)

    obj.box("MetalDark", (-30, -30, 0), (30, 30, 10))
    obj.cylinder_z("Yellow", 0, 0, 0, 140, 7, segments=10)
    obj.cylinder_z("Yellow", 0, 0, 136, 146, 16, segments=10)
    return obj


def build_prybar():
    obj = ObjBuilder("SM_Tool_PryBar")
    obj.use("Metal", METAL)
    obj.use("HandleRed", TOOLBOX_RED)

    obj.box("Metal", (-40, -4, 2), (36, 4, 10))
    obj.box("Metal", (30, -4, 8), (52, 4, 15), pitch=math.radians(-38))
    obj.box("HandleRed", (-52, -5, 1), (-38, 5, 11))
    return obj


def build_medkit():
    obj = ObjBuilder("SM_Tool_Medkit")
    obj.use("MedkitWhite", MEDKIT_WHITE)
    obj.use("MedkitRed", MEDKIT_RED)

    obj.box("MedkitWhite", (-20, -13, 0), (20, 13, 26))
    obj.box("MedkitRed", (-3, -9, 26), (3, 9, 27.5))
    obj.box("MedkitRed", (-9, -3, 26.2), (9, 3, 27.7))
    obj.box("MedkitWhite", (-24, -10, 10), (-20, 10, 16))
    return obj


def build_rope():
    obj = ObjBuilder("SM_Tool_Rope")
    obj.use("Yellow", YELLOW)
    obj.use("RopeEnd", (0.75, 0.62, 0.08))

    segments = 14
    radius = 18
    for i in range(segments):
        angle = TWO_PI * i / segments
        cx = radius * math.cos(angle)
        cy = radius * math.sin(angle)
        obj.box("Yellow", (cx - 5, cy - 4, 0), (cx + 5, cy + 4, 14), yaw=angle + math.pi * 0.5)
    obj.box("RopeEnd", (18, -4, 3), (48, 4, 11))
    return obj


def build_alert_board():
    obj = ObjBuilder("SM_AlertBoard")
    obj.use("BoardBody", BOARD_BODY)
    obj.use("BoardScreen", BOARD_SCREEN)
    obj.use("BoardTrim", BOARD_TRIM)
    obj.use("MetalDark", METAL_DARK)

    obj.box("MetalDark", (-84, -5, 0), (-72, 5, 60))
    obj.box("MetalDark", (72, -5, 0), (84, 5, 60))
    obj.box("BoardBody", (-100, -9, 55), (100, 9, 158))
    obj.box("BoardTrim", (-100, -11, 146), (100, 11, 158))
    obj.box("BoardScreen", (-90, -11, 68), (90, -9, 142))
    obj.box("BoardScreen", (-90, 9, 68), (90, 11, 142))
    return obj


def build_locker():
    obj = ObjBuilder("SM_Locker")
    obj.use("LockerGreen", LOCKER_GREEN)
    obj.use("LockerDark", LOCKER_DARK)
    obj.use("Metal", METAL)

    obj.box("LockerGreen", (-50, -35, 0), (50, 35, 185))
    obj.box("LockerDark", (-51, -36, 185), (51, 36, 190))
    obj.box("LockerDark", (-1.5, -36.5, 6), (1.5, 36.5, 180))
    obj.box("Metal", (-12, -40, 85), (0, -36, 108))
    obj.box("LockerDark", (-30, -36.5, 148), (30, 36.5, 154))
    obj.box("LockerDark", (-30, -36.5, 160), (30, 36.5, 166))
    return obj


def build_workshop():
    obj = ObjBuilder("SM_Workshop")
    obj.use("WoodTan", WOOD_TAN)
    obj.use("WoodLight", WOOD_LIGHT)
    obj.use("ToolboxRed", TOOLBOX_RED)
    obj.use("Metal", METAL)

    obj.box("WoodTan", (-160, -110, 0), (160, 110, 92))
    obj.box("WoodLight", (-170, -120, 92), (170, 120, 108))
    obj.box("ToolboxRed", (-145, -95, 108), (-75, -35, 148))
    obj.box("Metal", (-122, -68, 148), (-98, -62, 155))
    obj.box("Metal", (70, -95, 108), (130, -45, 128))
    obj.box("Metal", (78, -68, 128), (86, -62, 136))
    return obj


def build_button():
    obj = ObjBuilder("SM_Button")
    obj.use("ButtonBase", BUTTON_BASE)
    obj.use("Metal", METAL)
    obj.use("ButtonRed", BUTTON_RED)

    obj.cylinder_z("ButtonBase", 0, 0, 0, 10, 22)
    obj.cylinder_z("Metal", 0, 0, 10, 24, 11)
    obj.cylinder_z("ButtonRed", 0, 0, 22, 34, 18)
    return obj


def build_zone_pad():
    obj = ObjBuilder("SM_ZonePad")
    obj.use("PadGreen", PAD_GREEN)
    obj.use("PadWhite", PAD_WHITE)

    obj.box("PadGreen", (-300, -300, 0), (300, 300, 12))
    obj.box("PadWhite", (-300, -300, 12), (300, -280, 14))
    obj.box("PadWhite", (-300, 280, 12), (300, 300, 14))
    obj.box("PadWhite", (-300, -300, 14), (-280, 300, 16))
    obj.box("PadWhite", (280, -300, 14), (300, 300, 16))
    obj.box("PadWhite", (-25, -150, 16), (25, 150, 17.8))
    obj.box("PadWhite", (-150, -25, 16), (150, 25, 18))
    return obj


def build_current_pad():
    obj = ObjBuilder("SM_CurrentPad")
    obj.use("PadBlueDark", PAD_BLUE_DARK)
    obj.use("PadWhite", PAD_WHITE)

    obj.box("PadBlueDark", (-400, -400, 0), (400, 400, 12))

    for y0 in (-180, 0, 180):
        obj.box("PadWhite", (-69, y0 + 37, 12), (59, y0 + 53, 15), yaw=math.radians(-45))
        obj.box("PadWhite", (-69, y0 - 53, 12), (59, y0 - 37, 15), yaw=math.radians(45))
    return obj


MODELS = (
    build_player,
    build_victim,
    build_truck,
    build_door,
    build_debris,
    build_anchor,
    build_prybar,
    build_medkit,
    build_rope,
    build_alert_board,
    build_locker,
    build_workshop,
    build_button,
    build_zone_pad,
    build_current_pad,
)


def main():
    print("Generando modelos en: " + OUT_DIR)
    for builder in MODELS:
        obj = builder()
        obj.write(OUT_DIR)
        print("  {0}: {1} vertices, {2} caras".format(obj.name, len(obj.vertices), len(obj.faces)))
    print("Listo. Importalos con Tools/ue_prototype_setup.py dentro del editor.")


if __name__ == "__main__":
    main()
