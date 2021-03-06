/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include file detailing the resource partitioning for ATF */

/* resources that are going to stay in secure partition */
sc_rsrc_t secure_rsrcs[] = {
  SC_R_MU_0A,
  SC_R_A53,
  SC_R_A53_0,
  SC_R_A53_1,
  SC_R_A53_2,
  SC_R_A53_3,
  SC_R_A72,
  SC_R_A72_0,
  SC_R_A72_1,
  SC_R_GIC,
  SC_R_GIC_SMMU,
  SC_R_CCI,
  SC_R_SYSTEM,
#ifdef SPD_trusty
  SC_R_CAAM_JR2,
  SC_R_CAAM_JR2_OUT,
  SC_R_CAAM_JR3,
  SC_R_CAAM_JR3_OUT,
#endif
  SC_R_IRQSTR_SCU2
};

/* resources that have register access for non-secure domain */
sc_rsrc_t ns_access_allowed[] = {
  SC_R_GIC,
  SC_R_GIC_SMMU,
  SC_R_CCI
};
