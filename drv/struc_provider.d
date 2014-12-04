/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/LIBSLABLIST.LICENSE
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/LIBSLABLIST.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2012 Nicholas Zivkovic. All rights reserved.
 * Use is subject to license terms.
 */

provider struc {
	probe add_begin(void *sl, uint64_t e, uint64_t r) :
		(void *sl, uint64_t e, uint64_t r);
	probe add_end(int);
	probe rem_begin(void *sl, uint64_t e, uint64_t r) :
		(void *sl, uint64_t e, uint64_t r);
	probe rem_end(int);
	probe foldr_begin();
	probe foldr_end();
	probe foldl_begin();
	probe foldl_end();
};

