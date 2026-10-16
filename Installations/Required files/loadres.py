import json
import shutil
import sys
import time
from pathlib import Path


def ask(prompt, default):
    d = Path(default)
    if d.exists():
        shown = f"{default}  (Enter to use)"
    else:
        shown = default
    raw = input(f"{prompt}\n: ").strip().strip('"').strip("'")
    return Path(raw) if raw else d


def main():
    script_dir = Path(__file__).resolve().parent

    def_plants = script_dir / "PLANTTYPES.json"
    def_sandbox = script_dir / "SANDBOX.json"

    if len(sys.argv) > 1:
        plant_path = Path(sys.argv[1])
    else:
        plant_path = ask("PLANTTYPES.json path:", def_plants)

    if len(sys.argv) > 2:
        sandbox_path = Path(sys.argv[2])
    else:
        sandbox_path = ask("SANDBOX.json path:", def_sandbox)

    if not plant_path.exists():
        sys.exit(f"{plant_path} not found")
    if not sandbox_path.exists():
        sys.exit(f"{sandbox_path} not found")

    print()
    with open(plant_path, encoding="utf-8-sig") as f:
        plants = json.load(f)
    with open(sandbox_path, encoding="utf-8-sig") as f:
        sandbox = json.load(f)

    new_groups = set()
    count = 0
    for obj in plants.get("objects", []):
        if obj.get("objclass") == "PlantType":
            count += 1
            for g in obj.get("objdata", {}).get("PlantResourceGroups", []):
                if g:
                    new_groups.add(g)

    def find_rgn(node):
        if isinstance(node, dict):
            if isinstance(node.get("ResourceGroupNames"), list):
                return node, "ResourceGroupNames"
            for v in node.values():
                res = find_rgn(v)
                if res[0]:
                    return res
        elif isinstance(node, list):
            for item in node:
                res = find_rgn(item)
                if res[0]:
                    return res
        return None, None

    parent, key = find_rgn(sandbox)
    if not parent:
        sys.exit("ResourceGroupNames not found in SANDBOX.json")

    old_groups = set(parent[key])
    if old_groups == new_groups:
        print("no changes found.")
        return

    added = sorted(new_groups - old_groups)
    removed = sorted(old_groups - new_groups)

    print(f"ResourceGroupNames: {len(old_groups)} -> {len(new_groups)}")
    if added:
        print(f"  + {len(added)} groups")
        for g in added[:10]:
            print(f"      {g}")
        if len(added) > 10:
            print(f"      ... and {len(added) - 10} more")
    if removed:
        print(f"  - {len(removed)} groups")
        for g in removed[:10]:
            print(f"      {g}")
        if len(removed) > 10:
            print(f"      ... and {len(removed) - 10} more")

    confirm = input("\napply? (y/N): ").strip().lower()
    if confirm != "y":
        print("aborted.")
        return

    backup = sandbox_path.with_name(f"{sandbox_path.name}.bak-{time.strftime('%Y%m%d-%H%M%S')}")
    shutil.copy2(sandbox_path, backup)

    parent[key] = sorted(new_groups)
    with open(sandbox_path, "w", encoding="utf-8") as f:
        json.dump(sandbox, f, indent="\t", ensure_ascii=False)

    print(f"\nUpdated. Backup saved as {backup.name}")


if __name__ == "__main__":
    main()