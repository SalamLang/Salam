#include "hashmap_custom.h"

/**
 *
 * @function hashmap_print_layout_attribute
 * @brief Print the hashmap of layout attributes
 * @params {ast_layout_attribute_t*} map - The hashmap to print
 * @returns {void}
 *
 */
void hashmap_print_layout_attribute(hashmap_layout_attribute_t *map)
{
	DEBUG_ME;
	printf("Hashmap length: %zu\n", map->length);
	if (map->length == 0)
	{
		printf("Hashmap is empty\n");
		return;
	}

	size_t map_capacity = map->capacity;

	for (size_t i = 0; i < map_capacity; i++)
	{
		hashmap_entry_t *entry = map->data[i];

		while (entry)
		{
			printf("[%zu] Key: %s, Value: ", i, entry->key);
			ast_layout_attribute_t *layout_attribute = entry->value;

			if (layout_attribute != NULL)
			{
				layout_attribute->print(layout_attribute);
			}
			else
			{
				printf("NULL\n");
			}

			entry = cast(hashmap_entry_t *, entry->next);
		}
	}
}

/**
 *
 * @function hashmap_destroy_layout_attribute
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_destroy_layout_attribute(hashmap_layout_attribute_t *map)
{
	DEBUG_ME;
	if (map != NULL)
	{
		if (map->data != NULL)
		{
			size_t map_capacity = map->capacity;

			for (size_t i = 0; i < map_capacity; i++)
			{
				hashmap_entry_t *entry = map->data[i];

				while (entry)
				{
					hashmap_entry_t *next = cast(hashmap_entry_t *, entry->next);

					memory_destroy(entry->key);

					ast_layout_attribute_t *layout_attribute = cast(ast_layout_attribute_t *, entry->value);
					if (layout_attribute != NULL)
					{
						layout_attribute->destroy(layout_attribute);
					}

					memory_destroy(entry);

					entry = next;
				}
			}

			memory_destroy(map->data);
		}

		memory_destroy(map);
	}
}

/**
 *
 * @function hashmap_create_layout_attribute
 * @brief Create a new hashmap of layout attributes
 * @params {size_t} capacity
 * @returns {hashmap_layout_attribute_t*}
 *
 */
hashmap_layout_attribute_t *hashmap_create_layout_attribute(size_t capacity)
{
	DEBUG_ME;
	hashmap_layout_attribute_t *map = cast(struct hashmap_t *, hashmap_create(capacity));

	map->print = cast(void (*)(void *), hashmap_print_layout_attribute);
	map->destroy = cast(void (*)(void *), hashmap_destroy_layout_attribute);

	return map;
}

/**
 *
 * @function hashmap_create_layout_attribute_style_state
 * @brief Create a new hashmap of layout style state attributes
 * @params {size_t} capacity
 * @returns {hashmap_layout_attribute_state_style_t*}
 *
 */
hashmap_layout_attribute_state_style_t *hashmap_create_layout_attribute_style_state(size_t capacity)
{
	DEBUG_ME;
	hashmap_layout_attribute_state_style_t *map = hashmap_create(capacity);

	map->print = cast(void (*)(void *), hashmap_print_layout_attribute_style_state);
	map->destroy = cast(void (*)(void *), hashmap_destroy_layout_attribute_style_state);

	return map;
}

/**
 *
 * @function hashmap_print_layout_attribute_style_state
 * @brief Print the hashmap of layout style state attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_print_layout_attribute_style_state(hashmap_layout_attribute_t *map)
{
	DEBUG_ME;
	printf("Hashmap style states length: %zu\n", map->length);

	if (map->length == 0)
	{
		printf("Hashmap style states is empty\n");
		return;
	}
}

/**
 *
 * @function hashmap_destroy_layout_attribute_style_state
 * @brief Destroy the hashmap of layout style state attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_destroy_layout_attribute_style_state(hashmap_layout_attribute_t *map)
{
	DEBUG_ME;
	if (map != NULL)
	{
		if (map->data != NULL)
		{
			size_t map_capacity = map->capacity;

			for (size_t i = 0; i < map_capacity; i++)
			{
				hashmap_entry_t *entry = map->data[i];

				while (entry)
				{
					hashmap_entry_t *next = cast(hashmap_entry_t *, entry->next);

					if (entry->key != NULL)
					{
						memory_destroy(entry->key);
					}

					ast_layout_style_state_t *value = entry->value;
					if (value != NULL)
					{
						value->destroy(value);
					}

					memory_destroy(entry);

					entry = next;
				}
			}

			memory_destroy(map->data);
		}

		memory_destroy(map);
	}
}

/**
 *
 * @function hashmap_layout_attribute_print
 * @brief Print the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_layout_attribute_print(hashmap_layout_attribute_t *map)
{
	DEBUG_ME;
	printf("Hashmap layout attributes length: %zu\n", map->length);

	if (map->length == 0)
	{
		printf("Hashmap layout attributes is empty\n");
		return;
	}
}

/**
 *
 * @function hashmap_layout_attribute_destroy
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_layout_attribute_destroy(hashmap_layout_attribute_t *map)
{
	DEBUG_ME;
	if (map != NULL)
	{
		if (map->data != NULL)
		{
			size_t map_capacity = map->capacity;

			for (size_t i = 0; i < map_capacity; i++)
			{
				hashmap_entry_t *entry = map->data[i];

				while (entry)
				{
					hashmap_entry_t *next = cast(hashmap_entry_t *, entry->next);

					if (entry->key != NULL)
					{
						memory_destroy(entry->key);
					}

					hashmap_layout_attribute_t *value = entry->value;
					if (value != NULL)
					{
						value->destroy(value);
					}

					memory_destroy(entry);

					entry = next;
				}
			}

			memory_destroy(map->data);
		}

		memory_destroy(map);
	}
}

/**
 *
 * @function hashmap_has_any_sub_value_layout_attribute_style_state
 * @brief Check if the hashmap has any sub value layout attribute style state
 * @params {hashmap_layout_attribute_t*} map
 * @returns {bool}
 *
 */
bool hashmap_has_any_sub_value_layout_attribute_style_state(hashmap_layout_attribute_t *map)
{
	DEBUG_ME;
	if (map != NULL)
	{
		if (map->data != NULL)
		{
			size_t map_capacity = map->capacity;

			for (size_t i = 0; i < map_capacity; i++)
			{
				hashmap_entry_t *entry = map->data[i];

				while (entry)
				{
					hashmap_entry_t *next = cast(hashmap_entry_t *, entry->next);

					ast_layout_style_state_t *value = entry->value;
					if (value != NULL)
					{
						if (ast_layout_style_state_has_any_sub_value(value))
						{
							return true;
						}
					}

					entry = next;
				}
			}
		}
	}

	return false;
}

/**
 *
 * @function hashmap_has_any_sub_value_layout_attribute
 * @brief Check if the hashmap has any sub value layout attribute
 * @params {hashmap_layout_attribute_t*} map
 * @returns {bool}
 *
 */
bool hashmap_layout_attribute_has_any_sub_value(hashmap_layout_attribute_t *map)
{
	DEBUG_ME;
	if (map != NULL)
	{
		if (map->data != NULL)
		{
			size_t map_capacity = map->capacity;

			for (size_t i = 0; i < map_capacity; i++)
			{
				hashmap_entry_t *entry = map->data[i];

				while (entry)
				{
					hashmap_entry_t *next = cast(hashmap_entry_t *, entry->next);

					ast_layout_attribute_t *value = entry->value;
					if (value != NULL)
					{
						if (ast_layout_attribute_has_any_sub_value(value))
						{
							return true;
						}
					}

					entry = next;
				}
			}
		}
	}

	return false;
}
