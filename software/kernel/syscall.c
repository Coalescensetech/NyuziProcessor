//
// Copyright 2016 Jeff Bush
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "thread.h"
#include "libc.h"
#include "vga.h"

extern int user_copy(void *dest, const void *src, int count);

int handle_syscall(int arg0, int arg1, int arg2, int arg3, int arg4,
                   int arg5)
{
    char tmp[64];

    (void) arg4;
    (void) arg5;

    switch (arg0)
    {
        case 0: // Print something
            if ((unsigned int) arg2 >= sizeof(tmp) - 2)
            {
                kprintf("size out of range\n");
                return -1;
            }

            if (user_copy(tmp, (void*) arg1, arg2) < 0)
            {
                kprintf("user copy failed\n");
                return -1;
            }

            tmp[arg2] = '\0';
            kprintf("%s", tmp);
            return 0;

        case 1:
            spawn_user_thread((const char*) arg1, current_thread()->proc, arg2,
                              (void*) arg3);
            return 0;

        case 2: // Get thread ID
            return current_thread()->id;

        case 3: // Exec
        {
            // XXX unsafe user copy. Need copy_from_user
            struct process *proc = exec_program((const char*) arg1);
            if (proc)
                return proc->id;
            else
                return -1;
        }

        case 4: // Exit
            thread_exit(arg1);  // This will not return

        case 5: // Open VGA
            return (int) init_vga(arg1);

        case 6: // create_area
        {
            struct vm_area *area = create_area(current_thread()->proc->space,
                (unsigned int) arg1, // Address
                (unsigned int) arg2, // size
                arg3, // Placement
                arg4, // Name (XXX not safe)
                arg5, // flags,
                0, 0);

            if (area == 0)
                return 0;

            return area->low_address;
        }

        default:
            panic("Unknown syscall %d\n", arg0);
    }
}
