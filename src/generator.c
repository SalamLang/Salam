#include "generator.h"

/**
 * 
 * @function generator_create
 * @brief Create a generator
 * @params {}
 * @returns {generator_t*}
 * 
 */
generator_t* generator_create()
{
    generator_t* generator = memory_allocate(sizeof(generator_t));
    generator->html = string_create(4096);
    generator->css = string_create(4096);
    generator->js = string_create(4096);

    return generator;
}

/**
 * 
 * @function generator_destroy
 * @brief Destroy a generator
 * @params {generator_t*} generator
 * @returns {void}
 * 
 */
void generator_destroy(generator_t* generator)
{
    if (generator != NULL) {
        if (generator->html != NULL) {
            generator->html->destroy(generator->html);
        }

        if (generator->css != NULL) {
            generator->css->destroy(generator->css);
        }

        if (generator->js != NULL) {
            generator->js->destroy(generator->js);
        }

        memory_destroy(generator);
    }
}

/**
 * 
 * @function generator_debug
 * @brief Debug the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 * 
 */
void generator_debug(generator_t* generator)
{
    if (generator == NULL) {
        printf("generator is NULL\n");
        return;
    }

    printf("generator->html: ");
    if (generator->html != NULL) generator->html->print(generator->html);
    else printf("NULL\n");

    printf("generator->css: ");
    if (generator->css != NULL) generator->css->print(generator->css);
    else printf("NULL\n");

    printf("generator->js: ");
    if (generator->js != NULL) generator->js->print(generator->js);
    else printf("NULL\n");

}

/**
 * 
 * @function generator_save
 * @brief Save the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 * 
 */
void generator_save(generator_t* generator)
{
    if (generator == NULL) {
        return;
    }

    if (generator->html != NULL) {
        file_writes("index.html", generator->html);
    }

    if (generator->css != NULL) {
        file_writes("style.css", generator->css);
    }

    if (generator->js != NULL) {
        file_writes("script.js", generator->js);
    }
}
