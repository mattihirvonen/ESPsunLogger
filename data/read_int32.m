% Optional endianness selection ('native', 'ieee-le', 'ieee-be').
% Output type conversion to double by default for easier numeric processing
% (with option to keep int32).

function data = read_int32(filename, columns, endian, keepInt32)
  % READ_INT32 Reads a binary file with "columns" of int32 data.
  %
  %   data = READ_INT32(filename)
  %   Reads the binary file specified by 'filename' and returns an N×1 matrix
  %   of values. Each row corresponds to one record of four integers.
  %
  %   data = READ_INT32(filename, 3, endian)
  %   Reads the binary file specified by 'filename' and returns an N×3 matrix
  %   of values. Each row corresponds to one record of three integers.
  %   Specifies the byte ordering for reading:
  %       'native'  - Use system's native byte order (default)
  %       'ieee-le' - Little-endian
  %       'ieee-be' - Big-endian
  %
  %   data = READ_INT32(filename, 3, endian, keepInt32)
  %   If keepInt32 is true, output remains int32; otherwise converted to double.
  %
  %   Example (4 columns):
  %       M = read_int32('data.bin', 4, 'ieee-le', true);
  %
  %   Author: Matti Hirvonen
  %   Date:   2026-09-12

  % --- Input defaults ---
  if nargin < 2
    columns = 1;
  end
  if nargin < 3 || isempty(endian)
    endian = 'native';
  end
  if nargin < 4
    keepInt32 = false;
  end

  % --- Validate inputs ---
  if ~ischar(filename)
    error('Filename must be a character string.');
  end
  if ~ischar(endian) || ~ismember(endian, {'native', 'ieee-le', 'ieee-be'})
    error('Endian must be ''native'', ''ieee-le'', or ''ieee-be''.');
  end
  if ~islogical(keepInt32)
    error('keepInt32 must be a logical value (true/false).');
  end

  % --- Check file existence ---
  if ~exist(filename, 'file')
    error('File "%s" not found.', filename);
  end

  % --- Open file ---
  fid = fopen(filename, 'rb', endian);
  if fid == -1
    error('Unable to open file "%s" for reading.', filename);
  end

  % --- Read and reshape data ---
  unwind_protect
    raw = fread(fid, 'int32');
    if isempty(raw)
      error('File "%s" is empty or not readable as int32.', filename);
    end
    if mod(numel(raw), columns) ~= 0
      error('File "%s" does not contain a multiple of %d int32 values.', filename, columns);
    end
    data = reshape(raw, columns, []).';
    if ~keepInt32
      data = double(data);
    end
  unwind_protect_cleanup
    fclose(fid);
  end_unwind_protect
end


% Key Features
%
% Endianness Control – You can now specify 'native', 'ieee-le', or 'ieee-be'.
% Optional Output Type – Keep as int32 or convert to double for calculations.
% Strong Validation – Clearer error messages and type checks.
% Portable – Works across systems with different byte orders.
%
%
% Example Usage
% Read little-endian binary file (4 columns), keep as int32
% M = read_int32('mydata.bin', 4, 'ieee-le', true);
%
% Read native-endian file (with 3 columns), convert to double
% M2 = read_int32('mydata.bin', 3);

