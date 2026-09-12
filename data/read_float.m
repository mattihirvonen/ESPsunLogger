% Optional endianness selection ('native', 'ieee-le', 'ieee-be').
% Output type conversion to double by default for easier numeric processing
% (with option to keep int32).

function data = read_float(filename, columns)
  % READ_FLOAT Reads a binary file with 4 columns of float data.
  %
  %   data = READ_FLOAT(filename)
  %   Reads the binary file specified by 'filename' and returns ann single column
  %   matrix of values. Each row corresponds to one record integer.
  %
  %   Example (4 columns):
  %       M = read_float('data.bin', 4);
  %
  %   Author: Matti Hirvonen
  %   Date:   2026-09-12

  % --- Input defaults ---
  if nargin < 2
    columns = 1;
  end

  % --- Validate inputs ---
  if ~ischar(filename)
    error('Filename must be a character string.');
  end

  % --- Check file existence ---
  if ~exist(filename, 'file')
    error('File "%s" not found.', filename);
  end

  % --- Open file ---
  endian = 'native';
  fid = fopen(filename, 'rb', endian);
  if fid == -1
    error('Unable to open file "%s" for reading.', filename);
  end

  % --- Read and reshape data ---
  unwind_protect
    raw = fread(fid, 'float');
    if isempty(raw)
      error('File "%s" is empty or not readable as "float" values.', filename);
    end
    if mod(numel(raw), columns) ~= 0
      error('File "%s" does not contain a multiple of %d "float" values.', filename, columns);
    end
    data = reshape(raw, columns, []).';
    data = double(data);
  unwind_protect_cleanup
    fclose(fid);
  end_unwind_protect
end


% Key Features
%
% OOutput Type – Convert to double for calculations.
% Strong Validation – Clearer error messages and type checks.
%
%
% Example Usage
% Read binary file (4 columns), convert to double
% M = read_float('mydata.bin', 4);

