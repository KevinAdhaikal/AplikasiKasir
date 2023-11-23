/*
 * jQuery Weekpicker Addon
 * Author: Sander Marechal <s.marechal@jejik.com>
 *
 * Loosely based on the jquery timepicker addon by Trent Richardson [http://trentrichardson.com]
 *
 * Copyright 2014 Prezent Internet B.V.
 * You may use this product under the MIT license
 * http://opensource.org/licenses/MIT
 */

(function ($) {

    /*
     * Do not redefine weekpicker
     */
    $.ui.weekpicker = $.ui.weekpicker || {};
    if ($.ui.weekpicker.version) {
        return;
    }

    /*
     * Extend jQuery UI
     */
    $.extend($.ui, {
        weekpicker: {
            version: '@@version'
        }
    });

    /*
     * Weekpicker manager
     * Settings for the individual weekpickers are stored in an instance object
     */
    var Weekpicker = function () {
        this._defaults = {
            weekLength: 7,
            showWeek: true,
            startField: null,
            endField: null,
        };
    };

    $.extend(Weekpicker.prototype, {
        $input: null,
        startDate: null,
        endDate: null,

        /*
         * Override default settings for all weekpickers
         */
        setDefaults: function (settings) {
            $.extend(this._defaults, settings || {});
            return $this;
        },

        _setupWeekpicker: function ($dp) {
            $dp.addClass('ui-weekpicker');

            // Highlight the entire week row
            $dp.on('mousemove', '.ui-datepicker-calendar tr', function () {
                if ($(this).parents('.ui-weekpicker').length) {
                    $(this).find('td a').addClass('ui-state-hover');
                }
            });
            $dp.on('mouseleave', '.ui-datepicker-calendar tr', function () {
                if ($(this).parents('.ui-weekpicker').length) {
                    $(this).find('td a').removeClass('ui-state-hover');
                }
            });
        },

        /*
         * Create a new weekpicker instance
         */
        _newInst: function ($input, opts) {
            var wp_inst = new Weekpicker(),
                fns = {},
                overrides;

            /*
             * Override datepicker event functions. If the user has specified any in the options,
             * they are called at the end
             */
            overrides = {
                /*
                 * Add weekpicker class
                 */
                beforeShow: function (input, dp_inst) {
                    wp_inst._setupWeekpicker(dp_inst.dpDiv);
                    wp_inst._selectCurrentWeek();

                    if ($.isFunction(wp_inst._defaults.evnts.beforeShow)) {
                        return wp_inst._defaults.evnts.beforeShow.call($input[0], input, dp_inst);
                    }
                },

                /*
                 * Remove weekpicker class
                 */
                onClose: function (dateText, dp_inst) {
                    dp_inst.dpDiv.removeClass('ui-weekpicker');
                    $input.blur();

                    if ($.isFunction(wp_inst._defaults.evnts.onClose)) {
                        return wp_inst._defaults.evnts.onClose.call($input[0], dateText, dp_inst);
                    }
                },

                /*
                 * Select entire week instead of the day
                 */
                onSelect: function (dateText, dp_inst) {
                    wp_inst._setDates();
                    wp_inst._selectCurrentWeek();

                    if ($.isFunction(wp_inst._defaults.evnts.onSelect)) {
                        return wp_inst._defaults.evnts.onSelect.call($input[0], dateText, dp_inst);
                    }
                },

                /*
                 * Mark entire week as current day
                 */
                beforeShowDay: function (date) {
                    var cssClass = '';
                    if (date >= wp_inst.startDate && date <= wp_inst.endDate) {
                        cssClass = 'ui-datepicker-current-day';
                    }

                    var result = [true, cssClass];

                    if ($.isFunction(wp_inst._defaults.evnts.beforeShowDay)) {
                        $.each(wp_inst._defaults.evnts.beforeShowDay.call($input[0], date), function (i, v) {
                            if (i === 1) {
                                v = [v, cssClass].join(' ').trim();
                            }
                            result[i] = v;
                        });
                    }
                    return result;
                },
                
                /*
                 * Re-highlight week when flipping through the calendar
                 */
                onChangeMonthYear: function (year, month, dp_inst) {
                    wp_inst._selectCurrentWeek();

                    if ($.isFunction(wp_inst._defaults.evnts.onChangeMonthYear)) {
                        return wp_inst._defaults.evnts.onChangeMonthYear.call($input[0], year, month, dp_inst);
                    }
                }
            };

            for (i in overrides) {
                if (overrides.hasOwnProperty(i)) {
                    fns[i] = opts[i] || null;
                }
            }

            wp_inst._defaults = $.extend({}, this._defaults, opts, overrides, {
                evnts: fns,
                weekpicker: wp_inst // Allows $.datepicker._get(dp_inst, 'weekpicker')
            });

            wp_inst.$input = $input;
            console.log(wp_inst)
            return wp_inst;
        },

        /*
         * Highlight the entire week
         */
        _selectCurrentWeek: function () {
            window.setTimeout(function () {
                $('.ui-weekpicker').find('.ui-datepicker-current-day a').addClass('ui-state-active').removeClass('ui-state-default');
            }, 1);
        },

        /*
         * Set the start/end date
         */
        _setDates: function() {
            var dp_inst = this.$input.data('datepicker');
            var wp_inst = $.datepicker._get(dp_inst, 'weekpicker');
            var date = $.datepicker._getDate(dp_inst);

            if (date === null) {
                this.startDate = null;
                this.endDate = null;
                return;
            }

            var firstDay = this.$input.datepicker('option', 'firstDay');
            var offset = date.getDay() - firstDay;

            if (offset < 0) {
                offset += wp_inst._defaults.weekLength;
            }

            this.startDate = new Date(date.getFullYear(), date.getMonth(), date.getDate() - offset);
            this.endDate = new Date(date.getFullYear(), date.getMonth(), date.getDate() - offset + this._defaults.weekLength - 1);

            wp_inst._setDateFields();
        },

        /*
         * Update start/end form fields
         */
        _setDateFields: function () {
            var dp_inst = this.$input.data('datepicker');
            var wp_inst = $.datepicker._get(dp_inst, 'weekpicker');
            var dateFormat = dp_inst.settings.dateFormat || $.datepicker._defaults.dateFormat;

            // Update startField
            if (wp_inst._defaults.startField) {
                var date = $.datepicker.formatDate(dateFormat, wp_inst.startDate, dp_inst.settings);
                $(wp_inst._defaults.startField).val(date);

                if ($(wp_inst._defaults.startField)[0] == wp_inst.$input[0]) {
                    wp_inst.$input.datepicker('setDate', date);
                }
            }

            // Update endField
            if (wp_inst._defaults.endField) {
                var date = $.datepicker.formatDate(dateFormat, wp_inst.endDate, dp_inst.settings);
                $(wp_inst._defaults.endField).val(date);

                if ($(wp_inst._defaults.endField)[0] == wp_inst.$input[0]) {
                    wp_inst.$input.datepicker('setDate', date);
                }
            }
        }

    });

    $.fn.extend({

        /*
         * Shorthand to start weekpicker
         */
        weekpicker: function(o) {
            o = o || {};
            var args = arguments;

            if (typeof(o) === 'string') {
                // Forward function calls to datepicker
                if (o === 'getDate') {
                    return $.fn.datepicker.apply($(this[0]), args);
                } else {
                    return this.each(function () {
                        var $t = $(this);
                        $t.datepicker.apply($t, args);
                    });
                }
            } else {
                // Start weekpicker
                return this.each(function () {
                    var $t = $(this);
                    $t.datepicker($.weekpicker._newInst($t, o)._defaults);

                    var dp_inst = $t.data('datepicker');

                    if (dp_inst.inline) {
                        $.weekpicker._setupWeekpicker(dp_inst.dpDiv);
                    }
                });
            }
        }

    });

    /*
     * Create singleton manager
     */
    $.weekpicker = new Weekpicker();

    /*
     * Override to update the date range
     */
    $.datepicker._base_setDate = $.datepicker._setDate;
    $.datepicker._setDate = function (inst, date, noChange) {
        var result = this._base_setDate(inst, date, noChange);
        var wp_inst = this._get(inst, 'weekpicker');

        if (wp_inst) {
            wp_inst._setDates();
        }

        return result;
    };

    /*
     * Override to update the date range
     */
    $.datepicker._base_setDateFromField = $.datepicker._setDateFromField;
    $.datepicker._setDateFromField = function(inst, noDefault) {
        var result = this._base_setDateFromField(inst, noDefault);
        var wp_inst = this._get(inst, 'weekpicker');

        if (wp_inst) {
            wp_inst._setDates();
        }

        return result;
    };

    /*
     * Override to update the date range
     */
    $.datepicker._base_selectDay = $.datepicker._selectDay;
    $.datepicker._selectDay = function(id, month, year, td) {
        var inst = $.datepicker._getInst($(id)[0]);
        var result = this._base_selectDay(id, month, year, td);
        var wp_inst = this._get(inst, 'weekpicker');

        if (wp_inst) {
            wp_inst._setDates();
        }

        return result;
    };

    /*
     * Override to update the date range
     */
    $.datepicker._base_formatDate = $.datepicker._formatDate;
    $.datepicker._formatDate = function(inst, day, month, year) {
        var result = this._base_formatDate(inst, day, month, year);
        var wp_inst = this._get(inst, 'weekpicker');

        if (wp_inst) {
            wp_inst._setDates();
        }

        return result;
    };

})(jQuery);
