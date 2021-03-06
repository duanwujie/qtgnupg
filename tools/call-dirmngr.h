/* call-dirmngr.h - Interact with the Dirmngr.
 * Copyright (C) 2016 g10 Code GmbH
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GNUPG_TOOLS_CALL_DIRMNGR_H
#define GNUPG_TOOLS_CALL_DIRMNGR_H

void set_dirmngr_options (int verbose, int debug_ipc, int autostart);

gpg_error_t wkd_get_submission_address (const char *addrspec,
                                        char **r_addrspec);
gpg_error_t wkd_get_policy_flags (const char *addrspec, estream_t *r_buffer);


#endif /*GNUPG_TOOLS_CALL_DIRMNGR_H*/
