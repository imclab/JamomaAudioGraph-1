/* 
 *	adc≈
 *	Get input from A/D converter for Jamoma Multicore
 *	Copyright © 2010 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the GNU LGPL
 *	http://www.gnu.org/licenses/lgpl.html 
 */

#include "maxMulticore.h"


// Data Structure for this object
typedef struct Adc {
    Object					obj;
	TTMulticoreObjectPtr	multicoreObject;
	TTPtr					multicoreOutlet;
	SymbolPtr				attrMode;
};
typedef Adc* AdcPtr;


// Prototypes for methods
AdcPtr	AdcNew(SymbolPtr msg, AtomCount argc, AtomPtr argv);
void	AdcFree(AdcPtr self);
void	AdcAssist(AdcPtr self, void* b, long msg, long arg, char* dst);
TTErr	AdcReset(AdcPtr self);
TTErr	AdcSetup(AdcPtr self);
TTErr	AdcStart(AdcPtr self);
TTErr	AdcStop(AdcPtr self);
// Prototypes for attribute accessors
MaxErr	AdcSetSampleRate(AdcPtr self, void* attr, AtomCount argc, AtomPtr argv);
MaxErr	AdcGetSampleRate(AdcPtr self, void* attr, AtomCount* argc, AtomPtr* argv);
MaxErr	AdcSetVectorSize(AdcPtr self, void* attr, AtomCount argc, AtomPtr argv);
MaxErr	AdcGetVectorSize(AdcPtr self, void* attr, AtomCount* argc, AtomPtr* argv);


// Globals
static ClassPtr sAdcClass;


/************************************************************************************/
// Main() Function

int main(void)
{
	ClassPtr c;

	TTMulticoreInit();	
	common_symbols_init();

	c = class_new("jcom.adc≈", (method)AdcNew, (method)AdcFree, sizeof(Adc), (method)0L, A_GIMME, 0);
	
	class_addmethod(c, (method)AdcReset,			"multicore.reset",	A_CANT, 0);
	class_addmethod(c, (method)AdcSetup,			"multicore.setup",	A_CANT,	0);
	class_addmethod(c, (method)AdcStart,			"start",				0);
	class_addmethod(c, (method)AdcStop,				"stop",					0);
	class_addmethod(c, (method)AdcAssist,			"assist",			A_CANT, 0); 
    class_addmethod(c, (method)object_obex_dumpout,	"dumpout",			A_CANT, 0);  

	CLASS_ATTR_LONG(c,		"sampleRate",	0,		Adc,	obj);
	CLASS_ATTR_ACCESSORS(c,	"sampleRate",	AdcGetSampleRate,	AdcSetSampleRate);
	
	CLASS_ATTR_LONG(c,		"vectorSize",	0,		Adc,	obj);
	CLASS_ATTR_ACCESSORS(c,	"vectorSize",	AdcGetVectorSize,	AdcSetVectorSize);
	
	class_register(_sym_box, c);
	sAdcClass = c;
	return 0;
}


/************************************************************************************/
// Object Creation Method

AdcPtr AdcNew(SymbolPtr msg, AtomCount argc, AtomPtr argv)
{
    AdcPtr self = AdcPtr(object_alloc(sAdcClass));
	TTValue		v;
	TTErr		err;

    if (self) {
		v.setSize(2);
		v.set(0, TT("multicore.input"));
		v.set(1, TTUInt32(1));
		err = TTObjectInstantiate(TT("multicore.object"), (TTObjectPtr*)&self->multicoreObject, v);

		self->multicoreObject->addFlag(kTTMulticoreGenerator);

		attr_args_process(self, argc, argv);
    	object_obex_store((void*)self, _sym_dumpout, (object*)outlet_new(self, NULL));
		self->multicoreOutlet = outlet_new((t_pxobject*)self, "multicore.connect");
	}
	return self;
}

// Memory Deallocation
void AdcFree(AdcPtr self)
{
	TTObjectRelease((TTObjectPtr*)&self->multicoreObject);
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void AdcAssist(AdcPtr self, void* b, long msg, long arg, char* dst)
{
	if (msg==1)			// Inlets
		strcpy(dst, "multichannel audio connection and control messages");		
	else if (msg==2) {	// Outlets
		if (arg == 0)
			strcpy(dst, "multichannel audio connection");
		else
			strcpy(dst, "dumpout");
	}
}


TTErr AdcReset(AdcPtr self)
{
	return self->multicoreObject->reset();
}


TTErr AdcSetup(AdcPtr self)
{
	Atom a[2];
	
	atom_setobj(a+0, ObjectPtr(self->multicoreObject));
	atom_setlong(a+1, 0);
	outlet_anything(self->multicoreOutlet, gensym("multicore.connect"), 2, a);
	return kTTErrNone;
}


TTErr AdcStart(AdcPtr self)
{
	return self->multicoreObject->mUnitGenerator->sendMessage(TT("start"));
}


TTErr AdcStop(AdcPtr self)
{	
	return self->multicoreObject->mUnitGenerator->sendMessage(TT("stop"));
}


MaxErr AdcSetSampleRate(AdcPtr self, void* attr, AtomCount argc, AtomPtr argv)
{
	if (argc) {
		TTUInt32 sr = atom_getlong(argv);
		self->multicoreObject->mUnitGenerator->setAttributeValue(TT("sampleRate"), sr);
	}
	return MAX_ERR_NONE;
}

MaxErr AdcGetSampleRate(AdcPtr self, void* attr, AtomCount* argc, AtomPtr* argv)
{
	long sr;
	
	self->multicoreObject->mUnitGenerator->getAttributeValue(TT("sampleRate"), sr);
	
	*argc = 1;
	if (!(*argv)) // otherwise use memory passed in
		*argv = (t_atom *)sysmem_newptr(sizeof(t_atom));
	atom_setlong(*argv, sr);
	return MAX_ERR_NONE;
}


MaxErr AdcSetVectorSize(AdcPtr self, void* attr, AtomCount argc, AtomPtr argv)
{
	if (argc) {
		TTUInt32 vs = atom_getlong(argv);
		self->multicoreObject->mUnitGenerator->setAttributeValue(TT("vectorSize"), vs);
	}
	return MAX_ERR_NONE;
}

MaxErr AdcGetVectorSize(AdcPtr self, void* attr, AtomCount* argc, AtomPtr* argv)
{
	long vs;
	
	self->multicoreObject->mUnitGenerator->getAttributeValue(TT("vectorSize"), vs);
	
	*argc = 1;
	if (!(*argv)) // otherwise use memory passed in
		*argv = (t_atom *)sysmem_newptr(sizeof(t_atom));
	atom_setlong(*argv, vs);
	return MAX_ERR_NONE;
}

