extern "C"
{
	#include <jni.h>
	#include "core/logxcontroll.h"

	static jclass callbackClass;
	static jmethodID callbackJavaMethod;

	static JavaVM *jvm;

	static struct lxc_generic_porti_behavior java_bridge = {};

	struct java_bridge_instance
	{
		struct lxc_generic_porti_instance base;
		uint cb_length;
		jobject* callbacks;
	};

	static void java_bridge_execute(Gate instance, Signal type, LxcValue value, uint index)
	{
		if(NULL == type)
		{
			return;
		}

		struct java_bridge_instance* g = (struct java_bridge_instance*) instance;

		int abs = lxc_port_get_absindex(&(g->base.input_ports), type, index);
		if(abs > -1 || g->cb_length > abs)
		{
			jobject cb = g->callbacks[abs];
			if(NULL != cb)
			{
				JNIEnv *env;
				jint rs = jvm->AttachCurrentThread((void**)&env, NULL);
				if(JNI_OK == rs)
				{
					env->CallVoidMethod
					(
						cb,
						callbackJavaMethod,
						(long) instance,
						(long) type,
						index,
						(long) value
					);
				}
			}
		}
	}

	//TODO add/remove io ports

	static const char*** jb_path = (const char**[])
	{
		(const char*[])
		{"Communication", "Programming", NULL},
		NULL
	};

	static int java_library_operation(enum library_operation op, const char** error, int max_length)
	{
		if(library_before_load == op)
		{
			lxc_init_from_prototype
			(
				(void*) &java_bridge,
				sizeof(java_bridge),

				(void*) &lxc_generic_porti_prototype,
				sizeof(lxc_generic_porti_prototype)
			);

			java_bridge.base.gate_name = "java bridge";
			java_bridge.instance_memory_size = sizeof(struct java_bridge_instance);
			java_bridge.base.execute = java_bridge_execute;
			java_bridge.base.paths = jb_path;
			return 0;
		}

		return 0;
	}

	struct lxc_loadable_library java_library = {};

	static jclass classString;

	JNIEXPORT void JNICALL Java_eu_logxcontroll_LogxControll_initNative(JNIEnv * env,jobject obj)
	{
		jint rs = env->GetJavaVM(&jvm);
		if(JNI_OK != rs)
		{
			env->FatalError("Can't get JavaVM reference.");
		}

		logxcontroll_init_environment();

		memset((void*) &java_library, 0 , sizeof(java_library));
		java_library.library_operation = java_library_operation;

		array_pnt_append_element
		(
			(void***)	&(java_library.gates),
			(void*)		&java_bridge
		);

		const char* err[20];
		lxc_load_library(&java_library, err, 20);

		jclass cS = env->FindClass("java/lang/String");
		if(NULL == cS)
		{
			env->FatalError("Can't find java.lang.String class");
		}
		else
		{
			classString = env->NewGlobalRef(cS);
			env->DeleteLocalRef(cS);
		}

		jclass cC = env->FindClass("eu/logxcontroll/java/LogxControllCallback");
		if(NULL == cC)
		{
			env->FatalError("Can't find eu.logixcontroll.java.LogxControllCallback class");
		}
		else
		{
			callbackClass = env->NewGlobalRef(cC);
			env->DeleteLocalRef(cC);
		}

		callbackJavaMethod = env->GetMethodID(callbackClass, "callback", "(JJIJ)V");
		if(NULL == callbackJavaMethod)
		{
			env->FatalError("Can't find LogxControllCallback's callback(JJIJ)V function.");
		}
		else
		{
			//callbackJavaMethod = env->NewGlobalRef(callbackJavaMethod);
		}
	}

	JNIEXPORT void JNICALL Java_eu_logxcontroll_LogxControll_loadLogxControllSharedLibrary(JNIEnv * env,jobject obj, jstring file, jobjectArray arr)
	{
		if(NULL == file)
		{
			if(NULL != arr)
			{
				int len = env->GetArrayLength(arr);

				if(len > 0)
				{
					env->SetObjectArrayElement(arr, 0, (jobject) env->NewStringUTF("No shared library path specified."));
				}
			}

			return;
		}

		const char* path = env->GetStringUTFChars(file, NULL);

		int array_length = 0;
		const char** errors = NULL;

		if(NULL != arr)
		{
			array_length = env->GetArrayLength(arr);
			errors = (const char**) malloc_zero(sizeof(const char*)*array_length);
		}
		else
		{
			array_length = 10;
			errors = (const char**) malloc_zero(sizeof(const char*)*10);
		}

		int ret = lxc_load_shared_library(path, errors, array_length);

		env->ReleaseStringUTFChars(file, path);

		if(NULL != arr)
		{
			for(int i=0;i < array_length;++i)
			{
				if(NULL != errors[i])
				{
					env->SetObjectArrayElement(arr, i, env->NewStringUTF(errors[i]));
				}
				else
				{
					env->SetObjectArrayElement(arr, i, NULL);
				}
			}

			free(errors);
		}
	}

	JNIEXPORT int JNICALL Java_eu_logxcontroll_LogxControll_lxcValRefDiff(JNIEnv * env,jobject obj, long ptr, int ref_diff)
	{
		return lxc_refdiff_value((LxcValue) ptr, ref_diff);
	}

/*	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcGetWireValue(JNIEnv * env,jobject obj,long wire)
	{
		return (long) lxc_get_wire_value((Wire) wire);
	}
*/
	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcCreateGenericValue(JNIEnv * env,jobject obj,long signal, int size)
	{
		LxcValue val = lxc_create_generic_value((Signal) signal, size);
		return (long) val;
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcGetConstantByName(JNIEnv * env, jobject obj,jstring name)
	{
		const char* n = env->GetStringUTFChars(name, NULL);
		long ret = (long) lxc_get_constant_by_name(n);
		env->ReleaseStringUTFChars(name, n);
		return ret;
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcGetValueType(JNIEnv * env, jobject obj,long value)
	{
		return (long) (((LxcValue)value)->type);
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcGetValue(JNIEnv * env, jobject obj, long value)
	{
		return (long) (lxc_get_value((LxcValue) value));
	}


	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcGetSignalByName(JNIEnv * env, jobject obj,jstring name)
	{
		const char* n = env->GetStringUTFChars(name, NULL);
		long ret = (long) lxc_get_signal_by_name(n);
		env->ReleaseStringUTFChars(name, n);
		return ret;
	}

	JNIEXPORT void JNICALL Java_eu_logxcontroll_LogxControll_lxcDriveWireValue(JNIEnv * env, jobject obj,long instance, int outIndex, long wire, long value)
	{
		lxc_drive_wire_value((Gate) instance, outIndex, (Wire) wire, (LxcValue) value);
	}


	JNIEXPORT int JNICALL Java_eu_logxcontroll_LogxControll_lxcWireGateInput(JNIEnv * env, jobject obj, long signal, long wire, long gate, int index)
	{
		return lxc_wire_gate_input((Signal) signal, (Wire) wire, (Gate) gate, index);
	}

	JNIEXPORT int JNICALL Java_eu_logxcontroll_LogxControll_lxcWireGateOutput(JNIEnv * env, jobject obj, long signal, long wire, long gate, int index)
	{
		return lxc_wire_gate_output((Signal) signal, (Wire) wire, (Gate) gate, index);
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcCreateWire(JNIEnv * env, jobject obj, long signal)
	{
		return (long) lxc_create_wire((Signal) signal);
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcNewInstanceByName(JNIEnv * env, jobject obj, jstring name)
	{
		const char* n = env->GetStringUTFChars(name, NULL);
		long ret = (long) lxc_new_instance_by_name(n);
		env->ReleaseStringUTFChars(name, n);
		return ret;
	}

	JNIEXPORT bool JNICALL Java_eu_logxcontroll_LogxControll_lxcGateEnable(JNIEnv * env, jobject obj, long gate, bool set, bool enable)
	{
		if(set)
		{
			lxc_gate_set_enabled((Gate)gate, enable);
			return enable;
		}
		else
		{
			return lxc_gate_is_enabled((Gate) gate);
		}
	}

	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_lxcGetGatename(JNIEnv * env, jobject obj, long gate)
	{
		const char* name = lxc_get_gate_name((Gate) gate);
		if(NULL == name)
		{
			return NULL;
		}

		return env->NewStringUTF(name);
	}

	JNIEXPORT jobject JNICALL Java_eu_logxcontroll_LogxControll_lxcGetGateInputTypes(JNIEnv * env, jobject obj, long gate)
	{
		Signal sig[LXC_GATE_MAX_IO_TYPE_COUNT];
		int len = lxc_get_gate_input_types((Gate) gate, sig, LXC_GATE_MAX_IO_TYPE_COUNT);
		if(len < 0)
		{
			return NULL;
		}

		jlongArray ret = env->NewLongArray(len);
		long* arr = (long*) env->GetPrimitiveArrayCritical(ret, NULL);
		for(int i=0;i<len;++i)
		{
			arr[i] = (long) sig[i];
		}


		return ret;
	}

	JNIEXPORT jobject JNICALL Java_eu_logxcontroll_LogxControll_lxcGetGateIOTypes(JNIEnv * env, jobject obj, long gate, jboolean direction)
	{
		Signal sig[LXC_GATE_MAX_IO_TYPE_COUNT];
		int len = 	direction?
						lxc_get_gate_input_types((Gate) gate, sig, LXC_GATE_MAX_IO_TYPE_COUNT)
					:
						lxc_get_gate_output_types((Gate) gate, sig, LXC_GATE_MAX_IO_TYPE_COUNT);
		if(len < 0)
		{
			return NULL;
		}

		jlongArray ret = env->NewLongArray(len);
		long* arr = (long*) env->GetPrimitiveArrayCritical(ret, NULL);
		for(int i=0;i<len;++i)
		{
			arr[i] = (long) sig[i];
		}

		return ret;
	}

	JNIEXPORT int JNICALL Java_eu_logxcontroll_LogxControll_lxcGetPortMaxIndex(JNIEnv * env, jobject obj, long gate, long signal, jboolean direction)
	{
		return	direction?
					lxc_get_gate_input_max_index((Gate) gate, (Signal) signal)
				:
					lxc_get_gate_output_max_index((Gate) gate, (Signal) signal);
	}

	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_lxcGetIOLabel(JNIEnv * env, jobject obj, long gate, jboolean direction, long sig, int index)
	{
		const char* ret =
			direction?
				lxc_get_input_label((Gate) gate, (Signal) sig, index)
			:
				lxc_get_output_label((Gate)gate, (Signal) sig, index);

		if(NULL == ret)
		{
			return NULL;
		}

		return env->NewStringUTF(ret);
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcGetWire(JNIEnv * env, jobject obj, long gate, bool direction, long signal, int index)
	{
		if(direction == DIRECTION_IN)
		{
			return (long) lxc_get_input_wire((Gate) gate, (Signal) signal, index);
		}
		else
		{
			return (long) lxc_get_output_wire((Gate) gate, (Signal) signal, index);
		}
	}

	JNIEXPORT jarray JNICALL Java_eu_logxcontroll_LogxControll_lxcEnumerateProperties(JNIEnv * env, jobject obj, long gate)
	{
		const char* sarr[100];
		int len = lxc_enumerate_properties((Gate) gate, sarr, sizeof(sarr));

		if(len < 0)
		{
			len = 0;
		}

		jobjectArray arr = env->NewObjectArray(len, classString, 0);

		for(int i=0;i<len;++i)
		{
			if(NULL != sarr[i])
			{
				env->SetObjectArrayElement(arr, i, env->NewStringUTF(sarr[i]));
			}
		}

		return arr;
	}

	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_lxcGetPropertyLabel(JNIEnv * env, jobject obj, long gate, jstring prop)
	{
		const char* n = env->GetStringUTFChars(prop, NULL);
		const char* ret = lxc_get_property_label((Gate) gate, n);
		env->ReleaseStringUTFChars(prop, n);

		if(NULL == ret)
		{
			return NULL;
		}

		return env->NewStringUTF(ret);
	}

	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_lxcGetPropertyDescription(JNIEnv * env, jobject obj, long gate, jstring prop)
	{
		const char* n = env->GetStringUTFChars(prop, NULL);
		const char* ret = lxc_get_property_description((Gate) gate, n);
		env->ReleaseStringUTFChars(prop, n);

		if(NULL == ret)
		{
			return NULL;
		}

		return env->NewStringUTF(ret);
	}

	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_lxcGetPropertyValue(JNIEnv * env, jobject obj, long gate, jstring prop)
	{
		const char* n = env->GetStringUTFChars(prop, NULL);
		char ret[200];
		ret[199] = 0;
		int len = lxc_get_property_value((Gate) gate, n, ret, sizeof(ret));
		env->ReleaseStringUTFChars(prop, n);

		if(len < 0)
		{
			return NULL;
		}

		return env->NewStringUTF(ret);
	}

	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_lxcSetPropertyValue(JNIEnv * env, jobject obj, long gate, jstring prop, jstring value)
	{
		const char* n = env->GetStringUTFChars(prop, NULL);

		const char* v = NULL == value?NULL:env->GetStringUTFChars(value, NULL);

		char ret[200];
		ret[0] = 0;
		int re = lxc_set_property_value((Gate) gate, n, v, ret, sizeof(ret));
		env->ReleaseStringUTFChars(prop, n);

		if(NULL != value)
			env->ReleaseStringUTFChars(value, v);

		if(re == 0)
		{
			return NULL;
		}

		return env->NewStringUTF(ret);
	}

	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_lxcGetSignalName(JNIEnv * env, jobject obj, long sig)
	{
		return env->NewStringUTF(((Signal)sig)->name);
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcGetWireSignal(JNIEnv * env, jobject obj, long wire)
	{
		return (long) ((Wire) wire)->type;
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcPortiGetPortManager(JNIEnv * env, jobject obj, long gate, bool direction)
	{
		struct lxc_generic_porti_instance* i = (struct lxc_generic_porti_instance*) gate;
		return	direction?
					(long) &(i->input_ports)
				:
					(long) &(i->output_ports);
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcPortiGetWire(JNIEnv * env, jobject obj, long gate, bool direction, int abs)
	{
		struct lxc_generic_porti_instance* i = (struct lxc_generic_porti_instance*) gate;
		return	direction?
					(long) (i->inputs[abs])
				:
					(long) (i->outputs[abs]);
	}

	JNIEXPORT bool JNICALL Java_eu_logxcontroll_LogxControll_lxcCheckPortiInsance(JNIEnv * env, jobject obj, long gate)
	{
		Gate g = (Gate) gate;
		return lxc_generic_porti_prototype.base.get_input_label == g->behavior->get_input_label;
	}


	JNIEXPORT bool JNICALL Java_eu_logxcontroll_LogxControll_lxcPortCheckPortnameInUse(JNIEnv * env, jobject obj, long port_mngr, jstring name)
	{
		const char* n = env->GetStringUTFChars(name, NULL);
		bool ret = lxc_port_check_portname_in_use((struct lxc_port_manager*) port_mngr, n);
		env->ReleaseStringUTFChars(name, n);
		return ret;
	}

	JNIEXPORT void JNICALL Java_eu_logxcontroll_LogxControll_lxcPortUncheckedAddNewPort(JNIEnv * env, jobject obj, long pm, jstring name, long type)
	{
		const char* n = env->GetStringUTFChars(name, NULL);
		lxc_port_unchecked_add_new_port((struct lxc_port_manager*) pm, n, (Signal) type);
		env->ReleaseStringUTFChars(name, n);
	}

	JNIEXPORT void JNICALL Java_eu_logxcontroll_LogxControll_lxcPortUncheckedAddNewPort1(JNIEnv * env, jobject obj, long pm, long name, long type)
	{
		lxc_port_unchecked_add_new_port((struct lxc_port_manager*) pm, (const char*) name, (Signal) type);
	}

	JNIEXPORT int JNICALL Java_eu_logxcontroll_LogxControll_lxcPortGetAbsIndex(JNIEnv * env, jobject obj, long pm, long signal, int index)
	{
		return lxc_port_get_absindex((struct lxc_port_manager*) pm, (Signal) signal, index);
	}

	JNIEXPORT void JNICALL Java_eu_logxcontroll_LogxControll_lxcPortRemovePort(JNIEnv * env, jobject obj, long pm, long signal, int index)
	{
		lxc_port_remove_port((struct lxc_port_manager*) pm, (Signal) signal, index);
	}

	JNIEXPORT int JNICALL Java_eu_logxcontroll_LogxControll_lxcPortGetAbsindexByName(JNIEnv * env, jobject obj, long pm, jstring name)
	{
		const char* n = env->GetStringUTFChars(name, NULL);
		int ret = lxc_port_get_absindex_by_name((struct lxc_port_manager*) pm, n);
		env->ReleaseStringUTFChars(name, n);
		return ret;
	}

	JNIEXPORT jobject JNICALL Java_eu_logxcontroll_LogxControll_jbCb(JNIEnv * env, jobject obj, long gate, bool set, int index, jobject cb)
	{
		struct java_bridge_instance* g = (struct java_bridge_instance*) gate;

		if(set)
		{
			if(g->cb_length <= index)
			{
				array_fix_ensure_index
				(
					(void***) &(g->callbacks),
					&(g->cb_length),
					(uint) index
				);
			}

			fsync(1);

			jobject old = g->callbacks[index];
			if(NULL != old)
			{
				env->DeleteGlobalRef(old);
			}

			g->callbacks[index] = env->NewGlobalRef(cb);

			return NULL;
		}
		else
		{
			if(index < g->cb_length)
			{
				printf("get: %p\n", g->callbacks[index]);
				fsync(1);

				return g->callbacks[index];
			}

			return NULL;
		}
	}

	JNIEXPORT void JNICALL Java_eu_logxcontroll_LogxControll_lxcPortiReallocIOPort(JNIEnv * env, jobject obj, long gate, bool direction, int from, int to)
	{
		struct lxc_generic_porti_instance* g = (struct lxc_generic_porti_instance*) gate;
		void*** addr =	direction?
							(void***) &(g->inputs)
						:
							(void***) &(g->outputs);

		*addr = (void**) realloc((void*)*addr, sizeof(void*)*to);
		int diff = to-from;
		if(diff > 0)
		{
			memset(&((*addr)[from]), 0, sizeof(void*)*diff);
		}
	}

	JNIEXPORT int JNICALL Java_eu_logxcontroll_LogxControll_lxcPortManagerMaxIndex(JNIEnv * env, jobject obj, long pm)
	{
		return lxc_port_count((struct lxc_port_manager*)pm);
	}


	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_lxcLibTreeNodeName(JNIEnv * env, jobject obj, long tree_node)
	{
		if(0 == tree_node)
		{
			return NULL;
		}

		struct library_tree_node* ltn = (struct library_tree_node*) tree_node;
		return env->NewStringUTF(ltn->name);
	}

	JNIEXPORT jarray JNICALL Java_eu_logxcontroll_LogxControll_lxcLibTreeSubNode(JNIEnv * env, jobject obj, long tree_node)
	{
		struct library_tree_node** subs = ROOT_NODES;

		if(0 != tree_node)
		{
			struct library_tree_node* ltn = (struct library_tree_node*) tree_node;
			subs = ltn->subnodes;
		}

		int len = array_pnt_population((void**)subs);

		if(len <= 0)
		{
			return env->NewLongArray(0);
		}

		jlongArray ret = env->NewLongArray(len);
		long* data = (long*) env->GetLongArrayElements(ret, NULL);
		for(int i=0;i<len;++i)
		{
			data[i] = (long) subs[i];
		}

		env->ReleaseLongArrayElements(ret, data, 0);

		return ret;
	}


	JNIEXPORT jarray JNICALL Java_eu_logxcontroll_LogxControll_lxcLibTreeBehaviors(JNIEnv * env, jobject obj, long tree_node)
	{
		if(0 == tree_node)
		{
			return NULL;
		}

		struct library_tree_node* ltn = (struct library_tree_node*) tree_node;
		struct lxc_gate_behavior** bs = ltn->gates;
		int len = array_pnt_population((void**) bs);

		if(len <= 0)
		{
			return env->NewLongArray(0);
		}

		jlongArray ret = env->NewLongArray(len);
		long* data = (long*) env->GetLongArrayElements(ret, NULL);
		for(int i=0;i<len;++i)
		{
			data[i] = (long) bs[i];
		}

		env->ReleaseLongArrayElements(ret, data, 0);

		return ret;
	}

	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_lxcBehaviorGetName(JNIEnv * env, jobject obj, long behavior)
	{
		return env->NewStringUTF(((struct lxc_gate_behavior*) behavior)->gate_name);
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxcBehaviorNewInstance(JNIEnv * env, jobject obj, long behavior)
	{
		return (long) lxc_new_instance_by_behavior((const struct lxc_gate_behavior*) behavior);
	}

	JNIEXPORT jstring JNICALL Java_eu_logxcontroll_LogxControll_gnuLibcBacktraceSymbol(JNIEnv * env, jobject obj, long addr)
	{
		char sym[200];
		sym[0] = 0;
		gnu_libc_backtrace_symbol(addr, sym, sizeof(sym));
		return env->NewStringUTF(sym);
	}

	JNIEXPORT long JNICALL Java_eu_logxcontroll_LogxControll_lxc_get_token_value(JNIEnv * env, jobject obj, long addr)
	{
		return lxc_get_token_value((Tokenport) addr);
	}

	JNIEXPORT void JNICALL Java_eu_logxcontroll_LogxControll_lxc_absorb_token(JNIEnv * env, jobject obj, long addr)
	{
		lxc_absorb_token((Tokenport)addr);
	}
}
