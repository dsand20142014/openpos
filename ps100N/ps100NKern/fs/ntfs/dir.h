ud->id = (*id)++;
				}
				ud->lun = kv->value.immediate;
				ud->flags |= UNIT_DIRECTORY_HAS_LUN;

			/* Logical Unit Directory */
			} else if (kv->key.type == CSR1212_KV_TYPE_DIRECTORY) {
				/* This should really be done in SBP2 as this is
				 * doing SBP2 specific parsing.
				 */
				
				/* first register the parent unit */
				ud->flags |= UNIT_DIRECTORY_HAS_LUN_DIRECTORY;
				if (ud->device.bus != &ieee1394_bus_type)
					nodemgr_register_device(ne, ud, &ne->device);
				
				/* process the child unit */
				ud_child = nodemgr_process_unit_directory(ne, kv, id, ud);

				if (ud_child == NULL)
					break;
				
				/* inherit unspecified values, the driver core picks it up */
				if ((ud->flags & UNIT_DIRECTORY_MODEL_ID) &&
				    !(ud_child->flags & UNIT_DIRECTORY_MODEL_ID))
				{
					ud_child->flags |=  UNIT_DIRECTORY_MODEL_ID;
					ud_child->model_id = ud->model_id;
				}
				if ((ud->flags & UNIT_DIRECTORY_SPECIFIER_ID) &&
				    !(ud_child->flags & UNIT_DIRECTORY_SPECIFIER_ID))
				{
					ud_child->flags |=  UNIT_DIRECTORY_SPECIFIER_ID;
					ud_child->specifier_id = ud->specifier_id;
				}
				if ((ud->flags & UNIT_DIRECTORY_VERSION) &&
				    !(ud_child->flags & UNIT_DIRECTORY_VERSION))
				{
					ud_child->flags |=  UNIT_DIRECTORY_VERSION;
					ud_child->version = ud->version;
				}
				
				/* register the child unit */
				ud_child->flags |= UNIT_DIRECTORY_LUN_DIRECTORY;
				nodemgr_register_device(ne, ud_child, &ud->device);
			}

			break;

		case CSR1212_KV_ID_DIRECTORY_ID:
			ud->directory_id = kv->value.immediate;
			break;

		default:
			break;
		}
		last_key_id = kv->key.id;
	}
	
	/* do not process child units here and onl