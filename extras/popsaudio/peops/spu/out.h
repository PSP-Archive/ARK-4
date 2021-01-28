
struct out_driver {
	const char *name;
	int (*init)(void);
	void (*finish)(void);
	int (*busy)(void);
	void (*feed)(void *data, int bytes);
};

extern struct out_driver *out_current;

void SetupSound(void);
