# Sandbox Installation guide 101
## How 2 install sandbox mode

1. Firstly first, Encode [SANDBOX.json](https://github.com/aasu-rel/Soggylib/blob/main/Installations/Required%20files/SANDBOX.json) to rton and move it to packages/LEVELS
2. Copy the code in [ZOMBIETYPES](https://github.com/aasu-rel/Soggylib/blob/main/Installations/Required%20files/ZOMBIETYPES.json) and [ZOMBIEPROPERTIES](https://github.com/aasu-rel/Soggy/blob/main/Installations/Required%20files/ZOMBIEPROPERTIES.json) then place in your obb
3. put ZombieBlank.scg in packets (and edit obb's data.json)

Don't forget to add the way to enter sandbox level such as in worldmap

## Adding plants into sandbox mode
Just add plant's typename into PlantTyperOrder in PROPERTYSHEETS

Done, now plants in your mod should be loaded automatically in sandbox mode but zombies, You'll need to follow on how to add zombies in sandbox mode

## How 2 add zombies
This one is complicated, follow carefully

1. Find scg of zombie and unpack "for modding"
2. make a copy of animation folder
3. delete all animation labels (BESIDE IDLE)
4. delete all unnecessary sprites (butter and goo)
- Set their bitmap size to 0, 0 instead
5. copy whole idle animation more times and rename it to be idle1_1, attack, idle1_2
6. add new keyframe at the start of attack then press F9 and add this command
```AS3
fscommand("spawn_zombie", "Type:<ZombieType>, Team:Zombie");
fscommand("use_action", "");
```

### IMPORTANT
replace whole label structure in domdoc with this, delete other labels in label folder (except idle) and copy idle label 3 time and rename it to idle1_1, idle1_2, attack
```xml
<Include href="label/idle1_1.xml" itemIcon="1" loadImmediate="false" itemID="6acef1fb-000009ef"/>
<Include href="label/idle1_2.xml" itemIcon="1" loadImmediate="false" itemID="60a3f1fb-000009ef"/>
<Include href="label/attack.xml" itemIcon="1" loadImmediate="false" itemID="6acef50a-000009ef"/>
```

### COMMON ERROR
example error: `Image zombie_NAME_SIZExSIZE has no property "DOMBitmapInstance" in "elements"`

open `library/image/zombie_NAME_SIZExSIZE.xml` Inside the `<DOMSymbolItem>`'s timeline, find the frame's elements block.

### Before

```xml
<DOMFrame index="0" duration="1">
  <elements/>
</DOMFrame>
```

or the `<elements>` block is missing entirely.

### After

```xml
<DOMFrame index="0" duration="1">
  <elements>
    <DOMBitmapInstance libraryItemName="media/<symbol_name>">
      <matrix>
        <Matrix a="0" d="0" tx="0" ty="0"/>
      </matrix>
    </DOMBitmapInstance>
  </elements>
</DOMFrame>
```

7. edit data.json in your zombie scg

Open `data.json` in the unpacked SCG folder, add this in resource
```json
"POPANIM_ZOMBIE_ZOMBIE_TUTORIAL_DUMMY": {
					"type": "PopAnim",
					"path": "images/initial/zombie/zombie_tutorial_dummy"
				},
```
I hope you know what to change

8. repack the SCG as "for modding"
9. yeet the new SCG into your obb
10. make a new plant type
template:
```json
{
			"objclass": "PlantType",
			"aliases": [
				"dummytutorialbasic"
			],
			"objdata": {
				"TypeName": "dummytutorialbasic",
				"PlantFramework": "PlantDummyZombie",
				"PlantResourceGroups": [
					"ZombieTutorialGroup"
				],
				"Properties": "RTID(DummyZDefault@PlantProperties)",
				"AnimRigClass": "PlantAnimRig_DummyZombie",
				"PopAnim": "POPANIM_ZOMBIE_ZOMBIE_TUTORIAL_DUMMY",
				"SeedChooserVerticalOffset": 0.800000,
				"SeedChooserBigVerticalOffset": 0.360000,
				"SeedChooserBigHorizontalOffset": 0.000000,
				"SeedChooserBigScale": 1.200000,
				"SeasonsVerticalOffset": -25.000000,
				"Premium": true,
				"AlmanacBackdropName": "warp"
			}
		},
```

change only aliases, typename, resourcegroups and popanim, DON'T CHANGE FRAMEWORK AND ANIMRIG, everything else up to you

11. add it to the PlantTypeOrder (not PlantTypeMap) in PROPERTYSHEETS

```json
"PlantTypeOrder": [
    "newplantcool",
    "dummytutorialbasic",
    "dummytutorialbrick",
    "dummytutorialbucket",
    "dummytutorialcone"
]
```

12. add PlantResourceGroups in ResourceGroupNames in SANDBOX.json (must do else texture will be missing)

```json
"ResourceGroupNames": [
                    "PlantSunflower",
                    "PlantPeashooter",
					"etc",
					"ZombieTutorialGroup"
                ],
```

I know you won't do this manually so check python tool in [Required files](https://github.com/aasu-rel/Soggylib/blob/main/Installations/Required%20files/loadres.py)

boom done

---

## Common problems and how 2 fix them

**"Zombie shows in seed packet but placing does nothing"**
`attack` label is missing `fscommand` from step 5 and labels name have to be `idle1_1`, `attack`, `idle1_2`.

**"Crashes after placing/selecting"**
step 3 skipped. you didn't delete the other animation labels. The plant animrig problem.

**"Seed packet doesn't show"**
`PlantTypeOrder` wasn't updated (step 11).

---

## Pre answered questions

**"Why does it take so long to load level"**
Too many resources in level.json, no fix atm

**"Why don't you make it auto load resource group"**
BECAUSE I CAN'T, I'VE STUCK DOING THIS FOR WEEKS

**"Can I add zomboss?"**
No, It won't wake up

**"I found solution for resource loading"**
PLEASE TELL ME ABOUT IT, You'll be the greatest person ever in Soggylib history

## Credits

- **Blazey** - yummy sexy func
- **Snowie** - `fscommand("spawn_zombie")` action frame lib
- **Reno** - yummy headers and helps
- PvZ2 modding Discord - for answering at 3am
