#include "utils\memory.h"
#include "utils\imports.h"

NTSTATUS hook_handler(PVOID called_param)
{
	COPY_MEMORY* m = (COPY_MEMORY*)called_param;
	DbgPrintEx(0, 0, "got request\n");
	if (m->get_pid) {
		m->pid = memory::get_process_id(m->process_name);
	}
	else if (m->base) {
		DbgPrintEx(0, 0, "get base\n");
		PEPROCESS process;
		if (NT_SUCCESS(PsLookupProcessByProcessId(m->pid, &process))) {
			m->buffer = (void*)memory::get_module_base_x64(process);
		}
	}
	else if (m->peb) {
		PEPROCESS process;
		if (NT_SUCCESS(PsLookupProcessByProcessId(m->pid, &process))) {
			m->buffer = (void*)PsGetProcessPeb(process);
		}
	}
	else if (m->read) {
		DbgPrintEx(0, 0, "read memory\n");
		memory::read_kernel_memory(m->pid, (PVOID)m->address, m->buffer, m->size);
	}
	else if (m->write) {
		memory::write_kernel_memory(m->pid, m->buffer, (PVOID)m->address, m->size);
	}

	return STATUS_SUCCESS;
}

void real_entry() {
	DbgPrintEx(0, 0, "ApexDriver: Loaded.\n");

	if (memory::call_kernel_function(&hook_handler)) {
		DbgPrintEx(0, 0, "ApexDriver: function hooked - NtSetCompositionSurfaceIndependentFlipInfo");
	}
	else {
		DbgPrintEx(0, 0, "ApexDriver: failed to hook function.\n");
	}
}

extern "C" NTSTATUS EntryPoint(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
	UNREFERENCED_PARAMETER(driver_object);
	UNREFERENCED_PARAMETER(registry_path);
	DbgPrintEx(0, 0, "hook start.\n");
	real_entry();
	return STATUS_SUCCESS;
}