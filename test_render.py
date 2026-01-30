# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "numpy",
#     "pillow",
#     "platformdirs>=4.5.1",
#     "requests>=2.32.5",
# ]
# ///

"""Interactive test script for pystk2 render_data visualization.

Usage:
    PYTHONPATH=build uv run test_render.py [-k NUM_KARTS] [-t TRACK] [--no-display]

Options:
    -k, --num-karts    Total number of karts (default: 2)
    -c, --num-cameras  Number of cameras following top karts (0 = one per player)
    -t, --track        Track name (default: lighthouse)
    --no-display       Render offscreen only (no window update)
    --graphics         Graphics preset: hd, sd, ld (default: hd)

Controls:
    space / enter  Step once
    <number>       Step N times (e.g. 10, 50)
    c              Show color image
    d              Show depth image
    i              Show instance segmentation image
    a              Show all three (color, depth, instance)
    t              Tile all cameras into one image
    n / p          Next / previous camera
    q              Quit
"""

import argparse
import math
import sys
import io
import base64
import tty
import termios
import numpy as np
from PIL import Image

import pystk2


def imgcat(image: Image.Image, label: str = ""):
    """Display an image inline in iTerm2 using imgcat protocol."""
    buf = io.BytesIO()
    image.save(buf, format="PNG")
    data = base64.b64encode(buf.getvalue()).decode("ascii")
    name = base64.b64encode(label.encode()).decode("ascii") if label else ""
    sys.stdout.write(f"\033]1337;File=inline=1;name={name}:{data}\a\n")
    sys.stdout.flush()


def depth_to_image(depth: np.ndarray) -> Image.Image:
    d = np.asarray(depth, dtype=np.float32)
    lo, hi = np.min(d), np.max(d)
    if hi > lo:
        d = (d - lo) / (hi - lo)
    return Image.fromarray((d * 255).astype(np.uint8), mode="L")


def instance_to_image(instance: np.ndarray) -> Image.Image:
    inst = np.asarray(instance, dtype=np.uint32)
    r = ((inst * 37) & 0xFF).astype(np.uint8)
    g = ((inst * 91) & 0xFF).astype(np.uint8)
    b = ((inst * 159) & 0xFF).astype(np.uint8)
    return Image.fromarray(np.stack([r, g, b], axis=-1))


def read_key() -> str:
    """Read a single keypress or a sequence of digits followed by enter."""
    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        ch = sys.stdin.read(1)
        if ch.isdigit():
            # Accumulate digits until enter
            buf = ch
            while True:
                ch2 = sys.stdin.read(1)
                if ch2.isdigit():
                    buf += ch2
                else:
                    break
            return buf
        return ch
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old)


def show_color(rd, cam=0):
    if cam < len(rd):
        imgcat(Image.fromarray(np.array(rd[cam].image)), f"color_{cam}")


def show_depth(rd, cam=0):
    if cam < len(rd):
        imgcat(depth_to_image(np.array(rd[cam].depth)), f"depth_{cam}")


def show_instance(rd, cam=0):
    if cam < len(rd):
        imgcat(instance_to_image(np.array(rd[cam].instance)), f"instance_{cam}")


def tile_images(images: list[Image.Image]) -> Image.Image:
    """Tile a list of images into a grid."""
    n = len(images)
    if n == 0:
        return Image.new("RGB", (1, 1))
    cols = math.ceil(math.sqrt(n))
    rows = math.ceil(n / cols)
    w, h = images[0].size
    grid = Image.new("RGB", (cols * w, rows * h))
    for idx, img in enumerate(images):
        grid.paste(img, ((idx % cols) * w, (idx // cols) * h))
    return grid


def main():
    parser = argparse.ArgumentParser(description="Interactive pystk2 render test")
    parser.add_argument("-k", "--num-karts", type=int, default=2, help="Total number of karts (default: 2)")
    parser.add_argument("-c", "--num-cameras", type=int, default=0, help="Number of cameras following top karts (0 = one per player, default: 0)")
    parser.add_argument("-t", "--track", default="lighthouse", help="Track name (default: lighthouse)")
    parser.add_argument("--no-display", action="store_true", help="Use render=True, display=False mode")
    parser.add_argument("--graphics", choices=["hd", "sd", "ld"], default="hd", help="Graphics preset (default: hd)")
    parser.add_argument("--size", type=int, nargs=2, metavar=("WIDTH", "HEIGHT"), default=None, help="Render resolution (default: 600 400)")
    args = parser.parse_args()

    presets = {"hd": pystk2.GraphicsConfig.hd, "sd": pystk2.GraphicsConfig.sd, "ld": pystk2.GraphicsConfig.ld}
    print(f"Initializing pystk2 with {args.graphics.upper()} graphics...")
    gfx = presets[args.graphics]()
    if args.size:
        gfx.screen_width, gfx.screen_height = args.size
    if args.no_display:
        gfx.display = False
    pystk2.init(gfx)

    config = pystk2.RaceConfig(track=args.track, num_kart=args.num_karts)
    config.num_cameras = args.num_cameras
    config.players[0].controller = pystk2.PlayerConfig.Controller.AI_CONTROL
    config.players[0].camera_mode = pystk2.PlayerConfig.CameraMode.ON
    config.players[0].name = "Player 1"

    race = pystk2.Race(config)
    race.start()

    step_count = 0

    # Initial steps
    for _ in range(10):
        race.step()
        step_count += 1

    rd = race.render_data
    num_cams = len(rd)
    cam = 0
    print(f"render_data: {num_cams} camera(s)")
    if rd:
        arr = np.array(rd[0].image)
        print(f"  image shape: {arr.shape} dtype: {arr.dtype}")

    print(f"\n[step {step_count}, cam {cam}/{num_cams}]")
    print("  space=step  <N>=step N  d=depth  i=instance  c=color  a=all")
    print("  t=tile all  n/p=next/prev cam  q=quit")

    try:
        while True:
            sys.stdout.write(f"[step {step_count}, cam {cam}/{num_cams}]> ")
            sys.stdout.flush()
            key = read_key()
            sys.stdout.write("\n")

            if key in ("q", "\x03"):  # q or ctrl-c
                break
            elif key in (" ", "\r", "\n"):
                race.step()
                step_count += 1
                print(f"  step {step_count}")
            elif key.isdigit():
                n = int(key)
                for _ in range(n):
                    race.step()
                    step_count += 1
                print(f"  stepped {n} -> step {step_count}")
            elif key == "d":
                rd = race.render_data
                show_depth(rd, cam)
            elif key == "i":
                rd = race.render_data
                show_instance(rd, cam)
            elif key == "c":
                rd = race.render_data
                show_color(rd, cam)
            elif key == "a":
                rd = race.render_data
                print("  color:")
                show_color(rd, cam)
                print("  depth:")
                show_depth(rd, cam)
                print("  instance:")
                show_instance(rd, cam)
            elif key == "t":
                rd = race.render_data
                if rd:
                    imgs = [Image.fromarray(np.array(rd[j].image)) for j in range(len(rd))]
                    imgcat(tile_images(imgs), "tile")
            elif key == "n":
                cam = (cam + 1) % num_cams if num_cams > 0 else 0
                print(f"  cam {cam}")
            elif key == "p":
                cam = (cam - 1) % num_cams if num_cams > 0 else 0
                print(f"  cam {cam}")
            else:
                print(f"  unknown key: {repr(key)}")
                print("  space=step  <N>=step N  d=depth  i=instance  c=color  a=all")
                print("  t=tile all  n/p=next/prev cam  q=quit")
    except KeyboardInterrupt:
        pass

    print("\nStopping race...")
    race.stop()
    del race
    pystk2.clean()
    print("Done.")


if __name__ == "__main__":
    main()
