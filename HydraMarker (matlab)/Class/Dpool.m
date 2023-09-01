classdef Dpool
% tag pools

    properties
        val
        rot_prop
        %   val is a cell, which has the same row with shape, but has 4
        %   columns. The columns record all the possible tags of corresponding
        %   shape, where the tags in the 2-4 columns are the
        %   90бу/180бу/270бу-rotated versions of the ones in the first column.
        %   The elements that do not belong to the tags are -1.
        
        %   rot_prop indicates how the shapes repeat themselves.
        %   [1,2,3,4] means the tags in 1-4 columns of val have the same shape,
        %   which might confuse the reading algorithm (it happens when the
        %   shape repeats itself after 90бу rotation).
        %   [1,3;2,4] means the tags in 1 and 3 columns of val , 2 and 4 columns
        %   have the same shape (it happens when the shape repeats itself after
        %   at least 180бу rotation).
        %   [1;2;3;4] means the tags in the four columns of val all have different
        %   shapes.
    end
    
    methods
        function obj = Dpool(shape)
        % build a pool based on user-defined shapes
            %mustBeA(shape,'Dshape');
            
            snum = size(shape.val,1);
            obj.val = cell(snum,4);
            obj.rot_prop = cell(snum,1);
    
            %% find out all the possible tags
            for t = 1 : snum

            	[M,N] = size(shape.val{t});
                valid_ind = find(shape.val{t}==1);
                valid_length = size(valid_ind(:),1); % the number of valid entries
                valid_code = (double(dec2bin(0:2^valid_length-1))-48)'; % see ASCII for the reason of 48
                
                code = -ones(M*N,size(valid_code,2));
                code(valid_ind,:) = valid_code;

                obj.val{t,1} = reshape(code,[M,N,size(valid_code,2)]);
            end

            %% identify how the shapes repeat themselves
            for t = 1 : snum
                [shape_M,shape_N] = size(shape.val{t});
                if (shape_M == shape_N) && all(all(shape.val{t}==rot90(shape.val{t})))
                    obj.rot_prop{t} = [1,2,3,4];
                elseif all(all(shape.val{t}==rot90(shape.val{t},2)))
                    obj.rot_prop{t} = [1,3;2,4];
                else
                    obj.rot_prop{t} = [1;2;3;4];
                end   
            end

            %% remove the tags that conflict with themselves or others
            for t = 1 : snum
                [inst_M,inst_N,inst_num] = size(obj.val{t,1});
                inst = reshape(obj.val{t,1},[inst_M*inst_N,inst_num])';
                confuse_ind = zeros(inst_num,3);

                if size(obj.rot_prop{t},1) == 1
                    k_iter = 1 : 3;
                elseif  size(obj.rot_prop{t},1) == 2
                    k_iter = 2;
                else
                    k_iter = [];
                end

                for k = k_iter
                    inst_r = reshape(rot90(obj.val{t,1},k),[inst_M*inst_N,inst_num])';
                    [~,confuse_ind(:,k)] = ismember(inst,inst_r,'rows');
                end

                confuse_ind(confuse_ind < repmat((1:inst_num)',[1,3])) = NaN;

                confuse_ind = confuse_ind(:);
                confuse_ind(isnan(confuse_ind)) = [];
                obj.val{t,1}(:,:,confuse_ind) = [];

            end

            %% fill the 2-4 columns
            for t = 1 : size(shape.val,1)
                obj.val{t,2} = rot90(obj.val{t,1});
                obj.val{t,3} = rot90(obj.val{t,2});
                obj.val{t,4} = rot90(obj.val{t,3});
            end
        end % end function
        
    end % end method

end

