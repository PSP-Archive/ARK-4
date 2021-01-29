ifeq ($(V), 1)
	quiet = 
	Q =
else
	quiet = quiet_
	Q = @
endif

export quiet Q
