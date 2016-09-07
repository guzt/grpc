/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "src/core/ext/client_config/resolver_result.h"

#include <string.h>

#include <grpc/support/alloc.h>

grpc_addresses *grpc_addresses_create(size_t num_addresses) {
  grpc_addresses *addresses = gpr_malloc(sizeof(grpc_addresses));
  addresses->num_addresses = num_addresses;
  const size_t addresses_size = sizeof(grpc_address) * num_addresses;
  addresses->addresses = gpr_malloc(addresses_size);
  memset(addresses->addresses, 0, addresses_size);
  return addresses;
}

void grpc_addresses_set_address(grpc_addresses *addresses, size_t index,
                                void *address, size_t address_len,
                                bool is_balancer) {
  GPR_ASSERT(index < addresses->num_addresses);
  grpc_address *target = &addresses->addresses[index];
  memcpy(target->address.addr, address, address_len);
  target->address.len = address_len;
  target->is_balancer = is_balancer;
}

void grpc_addresses_destroy(grpc_addresses *addresses) {
  gpr_free(addresses->addresses);
  gpr_free(addresses);
}

struct grpc_resolver_result {
  gpr_refcount refs;
  grpc_lb_policy *lb_policy;
};

grpc_resolver_result *grpc_resolver_result_create() {
  grpc_resolver_result *c = gpr_malloc(sizeof(*c));
  memset(c, 0, sizeof(*c));
  gpr_ref_init(&c->refs, 1);
  return c;
}

void grpc_resolver_result_ref(grpc_resolver_result *c) { gpr_ref(&c->refs); }

void grpc_resolver_result_unref(grpc_exec_ctx *exec_ctx,
                                grpc_resolver_result *c) {
  if (gpr_unref(&c->refs)) {
    if (c->lb_policy != NULL) {
      GRPC_LB_POLICY_UNREF(exec_ctx, c->lb_policy, "resolver_result");
    }
    gpr_free(c);
  }
}

void grpc_resolver_result_set_lb_policy(grpc_resolver_result *c,
                                        grpc_lb_policy *lb_policy) {
  GPR_ASSERT(c->lb_policy == NULL);
  if (lb_policy) {
    GRPC_LB_POLICY_REF(lb_policy, "resolver_result");
  }
  c->lb_policy = lb_policy;
}

grpc_lb_policy *grpc_resolver_result_get_lb_policy(grpc_resolver_result *c) {
  return c->lb_policy;
}
