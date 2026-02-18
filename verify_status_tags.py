#!/usr/bin/env python3
"""
Verify that Go status tag constants match the C enum status_type definitions.
Reads src/status.h and compares with the tag constants in the Go code.
"""

import re
import sys

def parse_c_enum(filename):
    """Parse the C enum status_type from status.h"""
    tags = {}
    
    with open(filename, 'r') as f:
        content = f.read()
    
    # Find the enum status_type block
    enum_match = re.search(r'enum\s+status_type\s*\{([^}]+)\}', content, re.DOTALL)
    if not enum_match:
        print("ERROR: Could not find enum status_type in status.h")
        return None
    
    enum_body = enum_match.group(1)
    
    # Parse enum entries
    # Handle both explicit values (NAME = VALUE,) and implicit incrementing
    current_value = 0
    for line in enum_body.split('\n'):
        # Remove inline comments first
        line = re.sub(r'//.*$', '', line)
        line = line.strip()
        
        # Skip empty lines
        if not line:
            continue
        
        # Remove trailing comma
        line = line.rstrip(',').strip()
        
        if not line:
            continue
        
        # Check if explicit value is assigned
        if '=' in line:
            parts = line.split('=')
            name = parts[0].strip()
            value_str = parts[1].strip()
            try:
                current_value = int(value_str)
            except ValueError:
                # Skip if we can't parse the value
                print(f"Warning: Could not parse value for {name}: {value_str}")
                continue
            tags[name] = current_value
            current_value += 1
        else:
            # Implicit value (auto-increment)
            name = line.strip()
            if name:  # Only add non-empty names
                tags[name] = current_value
                current_value += 1
    
    return tags

def parse_go_tags(filename):
    """Parse Go tag constants from radiod_status.go"""
    tags = {}
    
    with open(filename, 'r') as f:
        content = f.read()
    
    # Find tag constant definitions
    # Pattern: tagName = VALUE // COMMENT
    pattern = r'tag(\w+)\s*=\s*(\d+)\s*//\s*(\w+)'
    
    for match in re.finditer(pattern, content):
        go_name = match.group(1)  # e.g., "FilterBlocksize"
        value = int(match.group(2))
        c_name = match.group(3)   # e.g., "FILTER_BLOCKSIZE"
        
        tags[c_name] = {
            'go_name': go_name,
            'value': value
        }
    
    return tags

def main():
    c_tags = parse_c_enum('src/status.h')
    if c_tags is None:
        sys.exit(1)
    
    go_tags = parse_go_tags('/home/nathan/repos/ka9q_ubersdr/radiod_status.go')
    
    print("=" * 80)
    print("STATUS TAG VERIFICATION")
    print("=" * 80)
    print()
    
    # Check the three new tags we added
    check_tags = ['FILTER_BLOCKSIZE', 'FILTER_FIR_LENGTH', 'FE_ISREAL']
    
    print("Checking newly added tags:")
    print("-" * 80)
    
    all_correct = True
    for c_name in check_tags:
        if c_name not in c_tags:
            print(f"❌ {c_name}: NOT FOUND in C enum")
            all_correct = False
            continue
        
        c_value = c_tags[c_name]
        
        if c_name not in go_tags:
            print(f"❌ {c_name}: NOT FOUND in Go constants")
            all_correct = False
            continue
        
        go_value = go_tags[c_name]['value']
        go_name = go_tags[c_name]['go_name']
        
        if c_value == go_value:
            print(f"✅ {c_name:25s} C={c_value:3d}  Go={go_value:3d}  (tag{go_name})")
        else:
            print(f"❌ {c_name:25s} C={c_value:3d}  Go={go_value:3d}  MISMATCH!")
            all_correct = False
    
    print()
    print("-" * 80)
    
    # Also check some known working tags for comparison
    print("\nVerifying some existing tags for comparison:")
    print("-" * 80)
    
    comparison_tags = ['INPUT_SAMPRATE', 'OUTPUT_SSRC', 'LOW_EDGE', 'HIGH_EDGE', 'KAISER_BETA', 'RF_GAIN', 'RF_ATTEN']
    
    for c_name in comparison_tags:
        if c_name in c_tags and c_name in go_tags:
            c_value = c_tags[c_name]
            go_value = go_tags[c_name]['value']
            go_name = go_tags[c_name]['go_name']
            
            if c_value == go_value:
                print(f"✅ {c_name:25s} C={c_value:3d}  Go={go_value:3d}  (tag{go_name})")
            else:
                print(f"❌ {c_name:25s} C={c_value:3d}  Go={go_value:3d}  MISMATCH!")
                all_correct = False
    
    print()
    print("=" * 80)
    
    if all_correct:
        print("✅ ALL TAGS MATCH CORRECTLY")
    else:
        print("❌ SOME TAGS DO NOT MATCH - FIX REQUIRED")
        sys.exit(1)
    
    print("=" * 80)
    print()
    
    # Print full C enum for reference
    print("\nFull C enum status_type from src/status.h:")
    print("-" * 80)
    for name in sorted(c_tags.keys(), key=lambda x: c_tags[x]):
        value = c_tags[name]
        go_info = ""
        if name in go_tags:
            go_info = f" → tag{go_tags[name]['go_name']}"
        print(f"{value:3d}: {name:30s}{go_info}")

if __name__ == '__main__':
    main()
