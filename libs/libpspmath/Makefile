TARGET_LIB=libpspmath.a

C_OBJS = \
printMatrixFloat.o \
vfpu_srand.o \
vfpu_randf.o \
vfpu_rand_8888.o \
vfpu_identity_matrix.o \
vfpu_translate_matrix.o \
vfpu_perspective_matrix.o \
vfpu_ortho_matrix.o \
vfpu_sinf.o \
vfpu_cosf.o \
vfpu_tanf.o \
vfpu_asinf.o \
vfpu_acosf.o \
vfpu_atanf.o \
vfpu_sinhf.o \
vfpu_coshf.o \
vfpu_tanhf.o \
vfpu_sincos.o \
vfpu_expf.o \
vfpu_logf.o \
vfpu_fabsf.o \
vfpu_sqrtf.o \
vfpu_powf.o \
vfpu_fmodf.o \
vfpu_fminf.o \
vfpu_fmaxf.o \
vfpu_ease_in_out.o \
vfpu_normalize_vector.o \
vfpu_zero_vector.o \
vfpu_scale_vector.o \
vfpu_add_vector.o \
vfpu_envmap_matrix.o \
vfpu_sphere_to_cartesian.o \
vfpu_quaternion_identity.o \
vfpu_quaternion_copy.o \
vfpu_quaternion_multiply.o \
vfpu_quaternion_normalize.o \
vfpu_quaternion_exp.o \
vfpu_quaternion_ln.o \
vfpu_quaternion_sample_linear.o \
vfpu_quaternion_from_euler.o \
vfpu_quaternion_to_matrix.o \
vfpu_quaternion_sample_hermite.o \
vfpu_quaternion_hermite_tangent.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -Os -G0 -Wall -fno-builtin-printf -I$(PSPSDK)/include
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =
LDFLAGS=-nodefaultlibs

include $(ARKROOT)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty_lib.mak
