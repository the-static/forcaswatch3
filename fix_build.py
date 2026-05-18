import sys

with open("src/c/layers/weather_status_layer.c", "r") as f:
    content = f.read()
content = content.replace("#define ARROW_H 11\n", "")
content = content.replace("#define ARROW_HEAD_H 4\n", "")
content = content.replace("#define ARROW_HEAD_W 3\n", "")
content = content.replace("#define ARROW_W 8\n", "")
with open("src/c/layers/weather_status_layer.c", "w") as f:
    f.write(content)

with open("src/c/layers/calendar_status_layer.c", "r") as f:
    content = f.read()
content = content.replace("((void)(bounds_h), (void)(icon_h), 0)", "0")
content = content.replace("((void)(bounds_h), 1)", "1")
with open("src/c/layers/calendar_status_layer.c", "w") as f:
    f.write(content)
