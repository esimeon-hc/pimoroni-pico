#include "drivers/servo/servo.hpp"
#include "drivers/servo/servo_cluster.hpp"
#include <cstdio>

#define MP_OBJ_TO_PTR2(o, t) ((t *)(uintptr_t)(o))

#define IS_SERVO_INVALID(servo)   (((servo) < 0) || ((servo) >= (int)NUM_BANK0_GPIOS))


using namespace servo;

extern "C" {
#include "servo.h"
#include "py/builtin.h"

typedef struct _mp_obj_float_t {
    mp_obj_base_t base;
    mp_float_t value;
} mp_obj_float_t;

const mp_obj_float_t const_float_1 = {{&mp_type_float}, 1.0f};

/********** Calibration **********/

/***** Variables Struct *****/
typedef struct _Calibration_obj_t {
    mp_obj_base_t base;
    Calibration *calibration;
    bool owner;
} _Calibtration_obj_t;


/***** Print *****/
void Calibration_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind; //Unused input parameter
    _Calibtration_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Calibtration_obj_t);
    Calibration* calib = self->calibration;
    mp_print_str(print, "Calibration(");

    uint size = calib->size();
    mp_print_str(print, "size = ");
    mp_obj_print_helper(print, mp_obj_new_int(size), PRINT_REPR);

    mp_print_str(print, ", points = {");
    for(uint i = 0; i < size; i++) {
        Calibration::Point *point = calib->point_at(i);
        mp_print_str(print, "{");
        mp_obj_print_helper(print, mp_obj_new_float(point->pulse), PRINT_REPR);
        mp_print_str(print, ", ");
        mp_obj_print_helper(print, mp_obj_new_float(point->value), PRINT_REPR);
        mp_print_str(print, "}");
        if(i < size - 1)
            mp_print_str(print, ", ");
    }
    mp_print_str(print, "})");
}

/***** Destructor ******/
mp_obj_t Calibration___del__(mp_obj_t self_in) {
    _Calibtration_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Calibtration_obj_t);
    if(self->owner)
        delete self->calibration;
    return mp_const_none;
}

/***** Constructor *****/
mp_obj_t Calibration_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    _Calibtration_obj_t *self = nullptr;

    enum { ARG_type };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_type, MP_ARG_INT, {.u_int = (uint8_t)servo::CalibrationType::ANGULAR} },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    servo::CalibrationType calibration_type = (servo::CalibrationType)args[ARG_type].u_int;

    self = m_new_obj_with_finaliser(_Calibtration_obj_t);
    self->base.type = &Calibration_type;

    self->calibration = new Calibration(calibration_type);
    self->owner = true;
    return MP_OBJ_FROM_PTR(self);
}

/***** Methods *****/
mp_obj_t Calibration_create_blank_calibration(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_size };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_size, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

    int size = args[ARG_size].u_int;
    if(size < 0)
        mp_raise_ValueError("size out of range. Expected 0 or greater");
    else
        self->calibration->create_blank_calibration((uint)size);

    return mp_const_none;
}

mp_obj_t Calibration_create_two_point_calibration(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_min_pulse, ARG_max_pulse, ARG_min_value, ARG_max_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_min_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_max_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_min_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_max_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

    float min_pulse = mp_obj_get_float(args[ARG_min_pulse].u_obj);
    float max_pulse = mp_obj_get_float(args[ARG_max_pulse].u_obj);
    float min_value = mp_obj_get_float(args[ARG_min_value].u_obj);
    float max_value = mp_obj_get_float(args[ARG_max_value].u_obj);
    self->calibration->create_two_point_calibration(min_pulse, max_pulse, min_value, max_value);

    return mp_const_none;
}

mp_obj_t Calibration_create_three_point_calibration(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_min_pulse, ARG_mid_pulse, ARG_max_pulse, ARG_min_value, ARG_mid_value, ARG_max_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_min_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_mid_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_max_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_min_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_mid_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_max_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

    float min_pulse = mp_obj_get_float(args[ARG_min_pulse].u_obj);
    float mid_pulse = mp_obj_get_float(args[ARG_mid_pulse].u_obj);
    float max_pulse = mp_obj_get_float(args[ARG_max_pulse].u_obj);
    float min_value = mp_obj_get_float(args[ARG_min_value].u_obj);
    float mid_value = mp_obj_get_float(args[ARG_mid_value].u_obj);
    float max_value = mp_obj_get_float(args[ARG_max_value].u_obj);
    self->calibration->create_three_point_calibration(min_pulse, mid_pulse, max_pulse, min_value, mid_value, max_value);

    return mp_const_none;
}

mp_obj_t Calibration_create_uniform_calibration(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_size, ARG_min_pulse, ARG_max_pulse, ARG_min_value, ARG_max_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_size, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_min_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_max_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_min_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_max_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

    int size = args[ARG_size].u_int;
    if(size < 0)
        mp_raise_ValueError("size out of range. Expected 0 or greater");
    else {
        float min_pulse = mp_obj_get_float(args[ARG_min_pulse].u_obj);
        float max_pulse = mp_obj_get_float(args[ARG_max_pulse].u_obj);
        float min_value = mp_obj_get_float(args[ARG_min_value].u_obj);
        float max_value = mp_obj_get_float(args[ARG_max_value].u_obj);
        self->calibration->create_uniform_calibration((uint)size, min_pulse, max_pulse, min_value, max_value);
    }

    return mp_const_none;
}

mp_obj_t Calibration_create_default_calibration(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_type };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_type, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

    servo::CalibrationType calibration_type = (servo::CalibrationType)args[ARG_type].u_int;
    self->calibration->create_default_calibration(calibration_type);

    return mp_const_none;
}

mp_obj_t Calibration_size(mp_obj_t self_in) {
    _Calibration_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Calibration_obj_t);
    return mp_obj_new_int(self->calibration->size());
}

mp_obj_t Calibration_point_at(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 2) {
        enum { ARG_self, ARG_index };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_index, MP_ARG_REQUIRED | MP_ARG_INT },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

        int index = args[ARG_index].u_int;
        if(index < 0 || index >= (int)self->calibration->size())
            mp_raise_ValueError("index out of range. Expected 0 to size()-1");
        else {
            Calibration::Point *point = self->calibration->point_at((uint)index);

            mp_obj_t tuple[2];
            tuple[0] = mp_obj_new_float(point->pulse);
            tuple[1] = mp_obj_new_float(point->value);
            return mp_obj_new_tuple(2, tuple);
        }
    }
    else {
        enum { ARG_self, ARG_index, ARG_point };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_index, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_point, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

        int index = args[ARG_index].u_int;
        if(index < 0 || index >= (int)self->calibration->size())
            mp_raise_ValueError("index out of range. Expected 0 to size()-1");
        else {
            Calibration::Point *point = self->calibration->point_at((uint)index);

            const mp_obj_t object = args[ARG_point].u_obj;
            if(mp_obj_is_type(object, &mp_type_list)) {
                mp_obj_list_t *list = MP_OBJ_TO_PTR2(object, mp_obj_list_t);
                if(list->len == 2) {
                    point->pulse = mp_obj_get_float(list->items[0]);
                    point->value = mp_obj_get_float(list->items[1]);
                }
                else {
                    mp_raise_ValueError("list must contain two numbers");
                }
            }
            else if(!mp_obj_is_type(object, &mp_type_tuple)) {
                mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR2(object, mp_obj_tuple_t);
                if(tuple->len == 2) {
                    point->pulse = mp_obj_get_float(tuple->items[0]);
                    point->value = mp_obj_get_float(tuple->items[1]);
                }
                else {
                    mp_raise_ValueError("tuple must contain two numbers");
                }
            }
            else {
                mp_raise_TypeError("can't convert object to list or tuple");
            }
        }

        return mp_const_none;
    }
}

mp_obj_t Calibration_first_point(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 1) {
        enum { ARG_self };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

        Calibration::Point *point = self->calibration->first_point();

        mp_obj_t tuple[2];
        tuple[0] = mp_obj_new_float(point->pulse);
        tuple[1] = mp_obj_new_float(point->value);
        return mp_obj_new_tuple(2, tuple);
    }
    else {
        enum { ARG_self, ARG_point };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_point, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

        Calibration::Point *point = self->calibration->first_point();

        const mp_obj_t object = args[ARG_point].u_obj;
        if(mp_obj_is_type(object, &mp_type_list)) {
            mp_obj_list_t *list = MP_OBJ_TO_PTR2(object, mp_obj_list_t);
            if(list->len == 2) {
                point->pulse = mp_obj_get_float(list->items[0]);
                point->value = mp_obj_get_float(list->items[1]);
            }
            else {
                mp_raise_ValueError("list must contain two numbers");
            }
        }
        else if(!mp_obj_is_type(object, &mp_type_tuple)) {
            mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR2(object, mp_obj_tuple_t);
            if(tuple->len == 2) {
                point->pulse = mp_obj_get_float(tuple->items[0]);
                point->value = mp_obj_get_float(tuple->items[1]);
            }
            else {
                mp_raise_ValueError("tuple must contain two numbers");
            }
        }
        else {
            mp_raise_TypeError("can't convert object to list or tuple");
        }

        return mp_const_none;
    }
}

mp_obj_t Calibration_last_point(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 1) {
        enum { ARG_self };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

        Calibration::Point *point = self->calibration->last_point();

        mp_obj_t tuple[2];
        tuple[0] = mp_obj_new_float(point->pulse);
        tuple[1] = mp_obj_new_float(point->value);
        return mp_obj_new_tuple(2, tuple);
    }
    else {
        enum { ARG_self, ARG_point };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_point, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

        Calibration::Point *point = self->calibration->last_point();

        const mp_obj_t object = args[ARG_point].u_obj;
        if(mp_obj_is_type(object, &mp_type_list)) {
            mp_obj_list_t *list = MP_OBJ_TO_PTR2(object, mp_obj_list_t);
            if(list->len == 2) {
                point->pulse = mp_obj_get_float(list->items[0]);
                point->value = mp_obj_get_float(list->items[1]);
            }
            else {
                mp_raise_ValueError("list must contain two numbers");
            }
        }
        else if(!mp_obj_is_type(object, &mp_type_tuple)) {
            mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR2(object, mp_obj_tuple_t);
            if(tuple->len == 2) {
                point->pulse = mp_obj_get_float(tuple->items[0]);
                point->value = mp_obj_get_float(tuple->items[1]);
            }
            else {
                mp_raise_ValueError("tuple must contain two numbers");
            }
        }
        else {
            mp_raise_TypeError("can't convert object to list or tuple");
        }

        return mp_const_none;
    }
}

mp_obj_t Calibration_limit_to_calibration(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_lower, ARG_upper };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_lower, MP_ARG_REQUIRED | MP_ARG_BOOL },
        { MP_QSTR_upper, MP_ARG_REQUIRED | MP_ARG_BOOL },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

    bool lower = args[ARG_lower].u_bool;
    bool upper = args[ARG_upper].u_bool;
    self->calibration->limit_to_calibration(lower, upper);

    return mp_const_none;
}

mp_obj_t Calibration_value_to_pulse(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

    float value = mp_obj_get_float(args[ARG_value].u_obj);

    float pulse_out, value_out;
    if(self->calibration->value_to_pulse(value, pulse_out, value_out)) {
        mp_obj_t tuple[2];
        tuple[0] = mp_obj_new_float(pulse_out);
        tuple[1] = mp_obj_new_float(value_out);
        return mp_obj_new_tuple(2, tuple);
    }
    else {
        mp_raise_msg(&mp_type_RuntimeError, "Unable to convert value to pulse. Calibration invalid");
    }
    return mp_const_none;
}

mp_obj_t Calibration_pulse_to_value(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_pulse };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _Calibration_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Calibration_obj_t);

    float pulse = mp_obj_get_float(args[ARG_pulse].u_obj);

    float value_out, pulse_out;
    if(self->calibration->pulse_to_value(pulse, value_out, pulse_out)) {
        mp_obj_t tuple[2];
        tuple[0] = mp_obj_new_float(pulse_out);
        tuple[1] = mp_obj_new_float(value_out);
        return mp_obj_new_tuple(2, tuple);
    }
    else {
        mp_raise_msg(&mp_type_RuntimeError, "Unable to convert pulse to value. Calibration invalid");
    }
    return mp_const_none;
}


/********** Servo **********/

/***** Variables Struct *****/
typedef struct _Servo_obj_t {
    mp_obj_base_t base;
    Servo* servo;
} _Servo_obj_t;


/***** Print *****/
void Servo_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind; //Unused input parameter
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    mp_print_str(print, "Servo(");

    mp_print_str(print, "pin = ");
    mp_obj_print_helper(print, mp_obj_new_int(self->servo->get_pin()), PRINT_REPR);
    mp_print_str(print, ", enabled = ");
    mp_obj_print_helper(print, self->servo->is_enabled() ? mp_const_true : mp_const_false, PRINT_REPR);
    mp_print_str(print, ", pulse = ");
    mp_obj_print_helper(print, mp_obj_new_float(self->servo->get_pulse()), PRINT_REPR);
    mp_print_str(print, ", value = ");
    mp_obj_print_helper(print, mp_obj_new_float(self->servo->get_value()), PRINT_REPR);
    mp_print_str(print, ", freq = ");
    mp_obj_print_helper(print, mp_obj_new_float(self->servo->get_frequency()), PRINT_REPR);

    mp_print_str(print, ")");
}

/***** Destructor ******/
mp_obj_t Servo___del__(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    delete self->servo;
    return mp_const_none;
}

/***** Constructor *****/
mp_obj_t Servo_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    _Servo_obj_t *self = nullptr;

    enum { ARG_pin, ARG_type };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_pin, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_type, MP_ARG_INT, {.u_int = (uint8_t)servo::CalibrationType::ANGULAR} },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int pin = args[ARG_pin].u_int;
    servo::CalibrationType calibration_type = (servo::CalibrationType)args[ARG_type].u_int;

    self = m_new_obj_with_finaliser(_Servo_obj_t);
    self->base.type = &Servo_type;

    self->servo = new Servo(pin, calibration_type);
    self->servo->init();

    return MP_OBJ_FROM_PTR(self);
}

/***** Methods *****/
extern mp_obj_t Servo_pin(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    return mp_obj_new_int(self->servo->get_pin());
}

extern mp_obj_t Servo_enable(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    self->servo->enable();
    return mp_const_none;
}

extern mp_obj_t Servo_disable(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    self->servo->disable();
    return mp_const_none;
}

extern mp_obj_t Servo_is_enabled(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    return self->servo->is_enabled() ? mp_const_true : mp_const_false;
}

extern mp_obj_t Servo_value(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 1) {
        enum { ARG_self };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Servo_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Servo_obj_t);

        return mp_obj_new_float(self->servo->get_value());
    }
    else {
        enum { ARG_self, ARG_value };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Servo_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Servo_obj_t);

        float value = mp_obj_get_float(args[ARG_value].u_obj);

        self->servo->set_value(value);
        return mp_const_none;
    }
}

extern mp_obj_t Servo_pulse(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 1) {
        enum { ARG_self };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Servo_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Servo_obj_t);

        return mp_obj_new_float(self->servo->get_pulse());
    }
    else {
        enum { ARG_self, ARG_pulse };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Servo_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Servo_obj_t);

        float pulse = mp_obj_get_float(args[ARG_pulse].u_obj);

        self->servo->set_pulse(pulse);
        return mp_const_none;
    }
}

extern mp_obj_t Servo_frequency(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 1) {
        enum { ARG_self };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Servo_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Servo_obj_t);

        return mp_obj_new_float(self->servo->get_frequency());
    }
    else {
        enum { ARG_self, ARG_freq };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_freq, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Servo_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Servo_obj_t);

        float freq = mp_obj_get_float(args[ARG_freq].u_obj);

        if(!self->servo->set_frequency(freq))
            mp_raise_ValueError("freq out of range. Expected 10Hz to 350Hz");
        else
            return mp_const_none;
    }
}

extern mp_obj_t Servo_min_value(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    return mp_obj_new_float(self->servo->get_min_value());
}

extern mp_obj_t Servo_mid_value(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    return mp_obj_new_float(self->servo->get_mid_value());
}

extern mp_obj_t Servo_max_value(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    return mp_obj_new_float(self->servo->get_max_value());
}

extern mp_obj_t Servo_to_min(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    self->servo->to_min();
    return mp_const_none;
}

extern mp_obj_t Servo_to_mid(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    self->servo->to_mid();
    return mp_const_none;
}

extern mp_obj_t Servo_to_max(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);
    self->servo->to_max();
    return mp_const_none;
}

extern mp_obj_t Servo_to_percent(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 2) {
        enum { ARG_self, ARG_in };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Servo_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Servo_obj_t);

        float in = mp_obj_get_float(args[ARG_in].u_obj);

        self->servo->to_percent(in);
    }
    else if(n_args <= 4) {
        enum { ARG_self, ARG_in, ARG_in_min, ARG_in_max };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in_min, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in_max, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Servo_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Servo_obj_t);

        float in = mp_obj_get_float(args[ARG_in].u_obj);
        float in_min = mp_obj_get_float(args[ARG_in_min].u_obj);
        float in_max = mp_obj_get_float(args[ARG_in_max].u_obj);

        self->servo->to_percent(in, in_min, in_max);
    }
    else {
        enum { ARG_self, ARG_in, ARG_in_min, ARG_in_max, ARG_value_min, ARG_value_max };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in_min, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in_max, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_value_min, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_value_max, MP_ARG_REQUIRED | MP_ARG_OBJ }
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _Servo_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _Servo_obj_t);

        float in = mp_obj_get_float(args[ARG_in].u_obj);
        float in_min = mp_obj_get_float(args[ARG_in_min].u_obj);
        float in_max = mp_obj_get_float(args[ARG_in_max].u_obj);
        float value_min = mp_obj_get_float(args[ARG_value_min].u_obj);
        float value_max = mp_obj_get_float(args[ARG_value_max].u_obj);

        self->servo->to_percent(in, in_min, in_max, value_min, value_max);
    }
    return mp_const_none;
}

extern mp_obj_t Servo_calibration(mp_obj_t self_in) {
    _Servo_obj_t *self = MP_OBJ_TO_PTR2(self_in, _Servo_obj_t);

    // NOTE This seems to work, in that it give MP access to the calibration object
    // Could very easily mess up in weird ways once object deletion is considered
    _Calibration_obj_t *calib = m_new_obj_with_finaliser(_Calibration_obj_t);
    calib->base.type = &Calibration_type;

    calib->calibration = &self->servo->calibration();
    calib->owner = false;

    return MP_OBJ_FROM_PTR(calib);
}


/********** ServoCluster **********/

/***** Variables Struct *****/
typedef struct _ServoCluster_obj_t {
    mp_obj_base_t base;
    ServoCluster* cluster;
} _ServoCluster_obj_t;


/***** Print *****/
void ServoCluster_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind; //Unused input parameter
    //_ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(self_in, _ServoCluster_obj_t);
    mp_print_str(print, "ServoCluster(");

    // TODO
    //mp_print_str(print, "num_leds = ");
    //mp_obj_print_helper(print, mp_obj_new_int(self->apa102->num_leds), PRINT_REPR);

    mp_print_str(print, ")");
}

/***** Destructor ******/
mp_obj_t ServoCluster___del__(mp_obj_t self_in) {
    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(self_in, _ServoCluster_obj_t);
    delete self->cluster;
    return mp_const_none;
}

/***** Constructor *****/
mp_obj_t ServoCluster_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    _ServoCluster_obj_t *self = nullptr;

    // TODO
    /*enum { 
        ARG_num_leds,
        ARG_pio,
        ARG_sm,
        ARG_dat,
        ARG_clk,
        ARG_freq,
        ARG_buffer
    };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_num_leds, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_pio, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_sm, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_dat, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_clk, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_freq, MP_ARG_INT, {.u_int = APA102::DEFAULT_SERIAL_FREQ} },
        { MP_QSTR_buffer, MP_ARG_OBJ, {.u_obj = nullptr} },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int num_leds = args[ARG_num_leds].u_int;
    PIO pio = args[ARG_pio].u_int == 0 ? pio0 : pio1;
    int sm = args[ARG_sm].u_int;
    int dat = args[ARG_dat].u_int;
    int clk = args[ARG_clk].u_int;
    int freq = args[ARG_freq].u_int;

    APA102::RGB *buffer = nullptr;

    if (args[ARG_buffer].u_obj) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_RW);
        buffer = (APA102::RGB *)bufinfo.buf;
        if(bufinfo.len < (size_t)(num_leds * 4)) {
            mp_raise_ValueError("Supplied buffer is too small for LED count!");
        }
        // If a bytearray is supplied it'll be raw, uninitialized bytes
        // iterate through the RGB elements and call "brightness"
        // to set up the SOF bytes, otherwise a flickery mess will happen!
        // Oh for such niceties as "placement new"...
        for(auto i = 0; i < num_leds; i++) {
            buffer[i].brightness(15);
        }
    }*/

    self = m_new_obj_with_finaliser(_ServoCluster_obj_t);
    self->base.type = &ServoCluster_type;

    self->cluster = new ServoCluster(pio1, 0, 0b11111100); //TODO Expose parameters

    return MP_OBJ_FROM_PTR(self);
}

/***** Methods *****/
extern mp_obj_t ServoCluster_pin_mask(mp_obj_t self_in) {
    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(self_in, _ServoCluster_obj_t);
    return mp_obj_new_int(self->cluster->get_pin_mask());
}

extern mp_obj_t ServoCluster_enable(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else
        self->cluster->enable((uint)servo);
    return mp_const_none;
}

extern mp_obj_t ServoCluster_disable(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else
        self->cluster->disable((uint)servo);
    return mp_const_none;
}

extern mp_obj_t ServoCluster_is_enabled(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else
        return self->cluster->is_enabled((uint)servo) ? mp_const_true : mp_const_false;
}

extern mp_obj_t ServoCluster_value(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 2) {
        enum { ARG_self, ARG_servo };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

        int servo = args[ARG_servo].u_int;
        if(IS_SERVO_INVALID(servo))
            mp_raise_ValueError("servo out of range. Expected 0 to 29");
        else
            return mp_obj_new_float(self->cluster->get_value((uint)servo));
    }
    else {
        enum { ARG_self, ARG_servo, ARG_value };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_value, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

        int servo = args[ARG_servo].u_int;
        if(IS_SERVO_INVALID(servo))
            mp_raise_ValueError("servo out of range. Expected 0 to 29");
        else {
            float value = mp_obj_get_float(args[ARG_value].u_obj);
            self->cluster->set_value((uint)servo, value);
        }
        return mp_const_none;
    }
}

extern mp_obj_t ServoCluster_pulse(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 2) {
        enum { ARG_self, ARG_servo };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

        int servo = args[ARG_servo].u_int;
        if(IS_SERVO_INVALID(servo))
            mp_raise_ValueError("servo out of range. Expected 0 to 29");
        else
            return mp_obj_new_float(self->cluster->get_pulse((uint)servo));
    }
    else {
        enum { ARG_self, ARG_servo, ARG_pulse };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_pulse, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

        int servo = args[ARG_servo].u_int;
        if(IS_SERVO_INVALID(servo))
            mp_raise_ValueError("servo out of range. Expected 0 to 29");
        else {
            float pulse = mp_obj_get_float(args[ARG_pulse].u_obj);
            self->cluster->set_pulse((uint)servo, pulse);
        }
        return mp_const_none;
    }
}

extern mp_obj_t ServoCluster_min_value(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else
        return mp_obj_new_float(self->cluster->get_min_value((uint)servo));
}

extern mp_obj_t ServoCluster_mid_value(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else
        return mp_obj_new_float(self->cluster->get_mid_value((uint)servo));
}

extern mp_obj_t ServoCluster_max_value(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else
        return mp_obj_new_float(self->cluster->get_max_value((uint)servo));
}

extern mp_obj_t ServoCluster_to_min(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else
        self->cluster->to_min((uint)servo);

    return mp_const_none;
}

extern mp_obj_t ServoCluster_to_mid(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else
        self->cluster->to_mid((uint)servo);

    return mp_const_none;
}

extern mp_obj_t ServoCluster_to_max(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else
        self->cluster->to_max((uint)servo);

    return mp_const_none;
}

extern mp_obj_t ServoCluster_to_percent(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    if(n_args <= 2) {
        enum { ARG_self, ARG_servo, ARG_in };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_in, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

        int servo = args[ARG_servo].u_int;
        if(IS_SERVO_INVALID(servo))
            mp_raise_ValueError("servo out of range. Expected 0 to 29");
        else {
            float in = mp_obj_get_float(args[ARG_in].u_obj);
            self->cluster->to_percent((uint)servo, in);
        }
    }
    else if(n_args <= 4) {
        enum { ARG_self, ARG_servo, ARG_in, ARG_in_min, ARG_in_max };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_in, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in_min, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in_max, MP_ARG_REQUIRED | MP_ARG_OBJ },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

        int servo = args[ARG_servo].u_int;
        if(IS_SERVO_INVALID(servo))
            mp_raise_ValueError("servo out of range. Expected 0 to 29");
        else {
            float in = mp_obj_get_float(args[ARG_in].u_obj);
            float in_min = mp_obj_get_float(args[ARG_in_min].u_obj);
            float in_max = mp_obj_get_float(args[ARG_in_max].u_obj);
            self->cluster->to_percent((uint)servo, in, in_min, in_max);
        }
    }
    else {
        enum { ARG_self, ARG_servo, ARG_in, ARG_in_min, ARG_in_max, ARG_value_min, ARG_value_max };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_in, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in_min, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_in_max, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_value_min, MP_ARG_REQUIRED | MP_ARG_OBJ },
            { MP_QSTR_value_max, MP_ARG_REQUIRED | MP_ARG_OBJ }
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

        int servo = args[ARG_servo].u_int;
        if(IS_SERVO_INVALID(servo))
            mp_raise_ValueError("servo out of range. Expected 0 to 29");
        else {
            float in = mp_obj_get_float(args[ARG_in].u_obj);
            float in_min = mp_obj_get_float(args[ARG_in_min].u_obj);
            float in_max = mp_obj_get_float(args[ARG_in_max].u_obj);
            float value_min = mp_obj_get_float(args[ARG_value_min].u_obj);
            float value_max = mp_obj_get_float(args[ARG_value_max].u_obj);
            self->cluster->to_percent((uint)servo, in, in_min, in_max, value_min, value_max);
        }
    }
    return mp_const_none;
}

extern mp_obj_t ServoCluster_calibration(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_servo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_servo, MP_ARG_REQUIRED | MP_ARG_INT },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _ServoCluster_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _ServoCluster_obj_t);

    int servo = args[ARG_servo].u_int;
    if(IS_SERVO_INVALID(servo))
        mp_raise_ValueError("servo out of range. Expected 0 to 29");
    else {
        // NOTE This seems to work, in that it give MP access to the calibration object
        // Could very easily mess up in weird ways once object deletion is considered
        _Calibration_obj_t *calib = m_new_obj_with_finaliser(_Calibration_obj_t);
        calib->base.type = &Calibration_type;

        calib->calibration = self->cluster->calibration((uint)servo);
        calib->owner = false;

        return MP_OBJ_FROM_PTR(calib);
    }

    return mp_const_none;
}
}