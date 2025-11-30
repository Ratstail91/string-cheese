# String Cheese

Using the rope pattern to build a small string library. It has an intended use-case within [Toy](https://github.com/krgamestudios/Toy).

Cue the cheesy jokes.

## Planned Features

* [X] allocate ropes
* [ ] release ropes
* [ ] split ropes
* [X] concat ropes
* [ ] insert (split, concat, concat)
* [ ] delete (split, split, concat)
* [ ] auto-balancing

## Known Issues

* [ ] All in `main.c` (will split it up later)
* [ ] No ref-counting (makes clearing unneeded entries easier)
* [ ] Limited capacity defined by `RESERVED_ROPES` and `RESERVED_STRINGS`
* [ ] Maximum string length is 1024 (could add another rope type for higher values)

## Testing

* [ ] Fuzzing
* [ ] Regression
* [ ] CI/CD