// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __IRR_MATH_H_INCLUDED__
#define __IRR_MATH_H_INCLUDED__

#include "irrTypes.h"

namespace irr
{
namespace core
{
	//! Rounding error constant often used when comparing f32 values.
	const f32 ROUNDING_ERROR	= 0.0001f;

	//! Constant for PI.
	const f32 PI				= 3.14159f;

	//! Constant for converting bettween degrees and radiants.
	const f64 GRAD_PI		= 180.0 / 3.1415926535897932384626433832795;

	//! Constant for converting bettween degrees and radiants.
	const f64 GRAD_PI2		= 3.1415926535897932384626433832795 / 180.0;

} // end namespace core
}// end namespace irr

#endif

