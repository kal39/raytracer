### Material Properties

```c
typedef struct Material {
	Color diffuse;
	float ambient;
	float specular;
	float specularExp;
	float reflective;
	float refractive;
	float rough;

} Material;
```

* `diffuse`: `Color`

  the colour that the object itself reflects

* `ambient`: `float` (0 ~ 1)

  how much light the object absorbs

* `specular`: `float` (0 ~ 1)

  how much highlights form light sources

* `specularExp`: `float` (1 ~ )

  highlights strength

* `reflective`: `float` (0 ~ 1)

  how much light the object reflects light from other objects

* `refractive`: `float` (0 ~ 1)

* `roughness`: float (0 ~ 1)

  how random reflections and refractions are

* (`luminosity`: ?)

* `transparency`: `float`

  how much light the object lets trough
