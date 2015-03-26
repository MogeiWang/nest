# nest
NEST, the neural network simulator, forked

### API documentation
http://synergetics.github.io/nest

### Creating a new model

For an example, see `models/simple.h` and `models/simple.cpp`

Other modifications include:

1. Register the new model with nest, in `modelsmodule.cpp`
```cpp
#include "simple.h"

// ...

register_model<simple>(net_,             "simple");

```

2. Add parameter names

`nest_names.h`
```cpp
    extern const Name spikes;

    extern const Name thresh;
```

`nest_names.cpp`
```cpp
    const Name spikes("spikes");

    const Name thresh("thresh");
```

3. Add entries in the makefiles

`Makefile.am`

```bash
    simple.h simple.cpp
```

`Makefile.in`

```bash
libmodelsmodule_la-simple.lo \

# ...

simple.h simple.cpp\

# ...

@AMDEP_TRUE@@am__include@ @am__quote@./$(DEPDIR)/libmodelsmodule_la-simple.Plo@am__quote@

# ...

libmodelsmodule_la-simple.lo: simple.cpp
@am__fastdepCXX_TRUE@ $(AM_V_CXX)$(LIBTOOL) $(AM_V_lt) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libmodelsmodule_la_CXXFLAGS) $(CXXFLAGS) -MT libmodelsmodule_la-simple.lo -MD -MP -MF $(DEPDIR)/libmodelsmodule_la-simple.Tpo -c -o libmodelsmodule_la-simple.lo `test -f 'simple.cpp' || echo '$(srcdir)/'`simple.cpp
@am__fastdepCXX_TRUE@ $(AM_V_at)$(am__mv) $(DEPDIR)/libmodelsmodule_la-simple.Tpo $(DEPDIR)/libmodelsmodule_la-simple.Plo
@AMDEP_TRUE@@am__fastdepCXX_FALSE@  $(AM_V_CXX)source='simple.cpp' object='libmodelsmodule_la-simple.lo' libtool=yes @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCXX_FALSE@  DEPDIR=$(DEPDIR) $(CXXDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCXX_FALSE@  $(AM_V_CXX@am__nodep@)$(LIBTOOL) $(AM_V_lt) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(libmodelsmodule_la_CXXFLAGS) $(CXXFLAGS) -c -o libmodelsmodule_la-simple.lo `test -f 'simple.cpp' || echo '$(srcdir)/'`simple.cpp
```


