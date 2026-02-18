# FM Squelch Disable Feature

## Overview

A new feature has been added to ka9q-radio to allow completely disabling the FM squelch by setting special threshold values.

## Usage

To disable FM squelch completely (keep it always open), set both squelch thresholds to `-999` or any value less than or equal to `-999`:

```ini
squelch-open = -999
squelch-close = -999
```

## How It Works

### Configuration Parsing (src/modes.c)

When the configuration is parsed, values of `-999` or less are detected and converted to a special marker value of `0.0` (power ratio):

```c
if(val <= -999.0f)
  chan->squelch_open = 0.0f; // Special marker for "always open"
else
  chan->squelch_open = dB2power(val);
```

This avoids the underflow issue where very negative dB values would convert to 0.0 anyway via `dB2power()`.

### FM Demodulator (src/fm.c)

The FM demodulator detects when both thresholds are set to the special `0.0` marker:

```c
bool squelch_always_open = (chan->squelch_open == 0.0f && chan->squelch_close == 0.0f);
```

When this flag is set:
1. The default value override logic is bypassed (lines 68-73)
2. The squelch comparison always evaluates to true (line 125)

### Wideband FM Demodulator (src/wfm.c)

The same logic is applied to the wideband FM demodulator for consistency.

## Implementation Details

### Files Modified

1. **src/modes.c** (lines 287-306)
   - Added special case detection for `-999` or less
   - Sets marker value of `0.0` instead of calling `dB2power()`

2. **src/fm.c** (lines 60-73, 125)
   - Added `squelch_always_open` flag detection
   - Bypasses default value override when flag is set
   - Adds flag to squelch open condition

3. **src/wfm.c** (lines 85-90, 137)
   - Same changes as fm.c for wideband FM

### Why -999?

The value `-999` was chosen because:
1. It's clearly outside the range of normal squelch values (-20 to +20 dB typical)
2. It's easy to remember and document
3. It would underflow to 0.0 via `dB2power()` anyway, so we intercept it
4. Any value ≤ -999 works, providing flexibility

### Backward Compatibility

This change is fully backward compatible:
- Existing configurations continue to work unchanged
- The special value `-999` is unlikely to have been used before
- Normal squelch values (e.g., -50, -100, 0, 8, 10) work as before

## Configuration Examples

### Preset File

```ini
[fm-nosquelch]
demod = fm
samprate = 24k
low = -8k
high = +8k
squelch-open = -999
squelch-close = -999
squelch-tail = 0
snr-squelch = no
tone = 0
threshold-extend = no
```

### Channel Configuration

```ini
[channel-cb]
freq = 27m185
mode = fm
squelch-open = -999
squelch-close = -999
```

### Runtime with control utility

The `control` utility can also set squelch to -999, though the configuration file is the recommended method.

## Testing

To verify the feature works:

1. Set `squelch-open = -999` and `squelch-close = -999`
2. Tune to a frequency with no signal (just noise)
3. You should hear continuous noise (squelch is open)
4. Check status - squelch should show as always open

## Limitations

### Tone Squelch Still Active

If you have PL/CTCSS tone squelch enabled (`tone`, `pl`, or `ctcss` parameter), it will still gate the audio even with carrier squelch disabled. To fully disable all squelching:

```ini
squelch-open = -999
squelch-close = -999
tone = 0  # Disable tone squelch
```

### SNR Reporting

When squelch is set to "always open", the SNR is still calculated and reported normally. This allows monitoring signal quality even with squelch disabled.

## Troubleshooting

### Squelch Still Closing

If squelch still closes after setting to -999:

1. **Check for tone squelch**: Ensure `tone`, `pl`, and `ctcss` are 0 or not set
2. **Verify configuration**: Use `control` utility to check actual running values
3. **Restart radiod**: Configuration changes require a restart
4. **Check both values**: Both `squelch-open` and `squelch-close` must be -999

### Status Shows "N/A"

If status shows squelch thresholds as "N/A", this is expected when using the special -999 value. The squelch is functioning correctly (always open).

## Future Enhancements

Possible future improvements:

1. Add explicit `squelch-disable = yes` configuration option
2. Add runtime command to toggle squelch on/off
3. Add status indicator showing "squelch disabled" vs "squelch open"
4. Consider separate control for carrier vs tone squelch

## Related Documentation

- [FM Squelch Analysis](fm_squelch_analysis.md) - Detailed analysis of squelch implementation
- [FM Squelch Troubleshooting](fm_squelch_troubleshooting.md) - Troubleshooting guide
- [docs/ka9q-radio-3.md](../docs/ka9q-radio-3.md) - Main configuration documentation
